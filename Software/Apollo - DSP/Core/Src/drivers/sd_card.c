#include "drivers/sd_card.h"

#include <stddef.h>

/* Spec allows 100 ms for a read token and 250 ms for a write; both are given
   headroom here because slow cards exist. */
#define SD_TOKEN_TIMEOUT_MS         250U
#define SD_BUSY_TIMEOUT_MS          600U
#define SD_INIT_TIMEOUT_MS          1200U
#define SD_IDLE_RETRY_LIMIT         16U

/* Bytes clocked with CS high so the card can synchronise before CMD0.
   The spec requires at least 74 clocks; 10 bytes gives 80. */
#define SD_POWERUP_PADDING_BYTES    10U

#define SD_DUMMY_BYTE               0xFFU

static uint32_t sd_prescaler_get(const SPI_HandleTypeDef *spi) {
	return (uint32_t)(spi->Instance->CR1 & SPI_CR1_BR);
}

/* Changing the baud rate requires the peripheral to be disabled, so this
   pokes CR1 directly rather than re-running HAL_SPI_Init on every call. */
static void sd_prescaler_set(SPI_HandleTypeDef *spi, uint32_t prescaler) {
	if (sd_prescaler_get(spi) == prescaler) {
		return;
	}

	__HAL_SPI_DISABLE(spi);
	MODIFY_REG(spi->Instance->CR1, SPI_CR1_BR, prescaler);
	spi->Init.BaudRatePrescaler = prescaler;
	__HAL_SPI_ENABLE(spi);
}

static void sd_select(sd_card_t *card) {
	HAL_GPIO_WritePin(card->cs_port, card->cs_pin, GPIO_PIN_RESET);
}

/* Deselect then clock one byte: the card needs an extra clock edge after CS
   rises before it stops driving MISO. */
static void sd_deselect(sd_card_t *card) {
	uint8_t dummy = SD_DUMMY_BYTE;

	HAL_GPIO_WritePin(card->cs_port, card->cs_pin, GPIO_PIN_SET);
	(void)HAL_SPI_Transmit(card->spi, &dummy, 1U, card->timeout_ms);
}

static apollo_status_t sd_xfer(sd_card_t *card, uint8_t tx, uint8_t *rx) {
	uint8_t rx_local = 0U;

	if (HAL_SPI_TransmitReceive(card->spi, &tx, &rx_local, 1U, card->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (rx != NULL) {
		*rx = rx_local;
	}

	return APOLLO_STATUS_OK;
}

static apollo_status_t sd_read_byte(sd_card_t *card, uint8_t *value) {
	return sd_xfer(card, SD_DUMMY_BYTE, value);
}

/* Clocks dummy bytes until the card stops holding the line low. */
static apollo_status_t sd_wait_not_busy(sd_card_t *card, uint32_t timeout_ms) {
	uint32_t start = HAL_GetTick();
	uint8_t value = 0U;

	do {
		if (sd_read_byte(card, &value) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		if (value == 0xFFU) {
			return APOLLO_STATUS_OK;
		}
	} while ((HAL_GetTick() - start) < timeout_ms);

	return APOLLO_STATUS_TIMEOUT;
}

/* Sends a command frame and returns the R1 byte. The card may issue up to
   eight 0xFF bytes before responding. */
static apollo_status_t sd_send_command(sd_card_t *card,
									   uint8_t command,
									   uint32_t argument,
									   uint8_t *response) {
	uint8_t frame[SD_COMMAND_FRAME_BYTES];
	uint8_t value = 0xFFU;

	sd_build_command(frame, command, argument);

	/* CMD12 is preceded by a stuff byte that the card discards. */
	if (command == SD_CMD12_STOP_TRANSMISSION) {
		(void)sd_read_byte(card, NULL);
	}

	if (HAL_SPI_Transmit(card->spi, frame, (uint16_t)sizeof(frame), card->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	for (uint8_t attempt = 0U; attempt < 10U; attempt++) {
		if (sd_read_byte(card, &value) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		if ((value & 0x80U) == 0U) {
			if (response != NULL) {
				*response = value;
			}
			return APOLLO_STATUS_OK;
		}
	}

	return APOLLO_STATUS_TIMEOUT;
}

/* CMD55 + the application command that follows it. */
static apollo_status_t sd_send_app_command(sd_card_t *card,
										   uint8_t command,
										   uint32_t argument,
										   uint8_t *response) {
	apollo_status_t status = sd_send_command(card, SD_CMD55_APP_CMD, 0UL, NULL);

	if (status != APOLLO_STATUS_OK) {
		return status;
	}

	return sd_send_command(card, command, argument, response);
}

/* Reads the trailing four bytes of an R3/R7 response. */
static apollo_status_t sd_read_r3_r7(sd_card_t *card, uint32_t *value) {
	uint32_t result = 0U;

	for (uint8_t index = 0U; index < 4U; index++) {
		uint8_t byte = 0U;

		if (sd_read_byte(card, &byte) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		result = (result << 8) | byte;
	}

	if (value != NULL) {
		*value = result;
	}

	return APOLLO_STATUS_OK;
}

/* Waits for a data token and reads the payload plus its CRC16. */
static apollo_status_t sd_read_data_block(sd_card_t *card, uint8_t *buffer, uint32_t length) {
	uint32_t start = HAL_GetTick();
	uint8_t token = 0U;
	uint8_t crc_bytes[2] = { 0U, 0U };
	uint16_t crc_received;
	uint16_t crc_expected;

	do {
		if (sd_read_byte(card, &token) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		if (token != 0xFFU) {
			break;
		}
	} while ((HAL_GetTick() - start) < SD_TOKEN_TIMEOUT_MS);

	if (token != SD_TOKEN_START_BLOCK) {
		return APOLLO_STATUS_TIMEOUT;
	}

	for (uint32_t index = 0U; index < length; index++) {
		buffer[index] = SD_DUMMY_BYTE;
	}
	if (HAL_SPI_TransmitReceive(card->spi, buffer, buffer, (uint16_t)length, card->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (sd_read_byte(card, &crc_bytes[0]) != APOLLO_STATUS_OK ||
		sd_read_byte(card, &crc_bytes[1]) != APOLLO_STATUS_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	crc_received = (uint16_t)(((uint16_t)crc_bytes[0] << 8) | crc_bytes[1]);
	crc_expected = sd_crc16(buffer, length);
	if (crc_received != crc_expected) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}

/* Sends one data token, its payload and CRC, then checks the data response. */
static apollo_status_t sd_write_data_block(sd_card_t *card,
										   uint8_t token,
										   const uint8_t *buffer,
										   uint32_t length) {
	uint16_t crc = sd_crc16(buffer, length);
	uint8_t crc_bytes[2] = { (uint8_t)(crc >> 8), (uint8_t)(crc & 0xFFU) };
	uint8_t response = 0U;

	if (sd_wait_not_busy(card, SD_BUSY_TIMEOUT_MS) != APOLLO_STATUS_OK) {
		return APOLLO_STATUS_TIMEOUT;
	}

	if (sd_xfer(card, token, NULL) != APOLLO_STATUS_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (token == SD_TOKEN_STOP_MULTI_WRITE) {
		return sd_wait_not_busy(card, SD_BUSY_TIMEOUT_MS);
	}

	if (HAL_SPI_Transmit(card->spi, (uint8_t *)buffer, (uint16_t)length, card->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (HAL_SPI_Transmit(card->spi, crc_bytes, 2U, card->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (sd_read_byte(card, &response) != APOLLO_STATUS_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if ((response & SD_DATA_RESPONSE_MASK) != SD_DATA_RESPONSE_ACCEPTED) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return sd_wait_not_busy(card, SD_BUSY_TIMEOUT_MS);
}

/* Drives CMD0/CMD8/ACMD41/CMD58 and classifies the card. Assumes the bus is
   already at the initialisation clock and the card is selected. */
static apollo_status_t sd_run_init_sequence(sd_card_t *card) {
	uint8_t response = 0U;
	uint32_t start;
	uint32_t ocr = 0U;
	uint32_t if_cond = 0U;
	uint8_t is_v2 = 0U;
	uint8_t entered_idle = 0U;

	for (uint8_t attempt = 0U; attempt < SD_IDLE_RETRY_LIMIT; attempt++) {
		if (sd_send_command(card, SD_CMD0_GO_IDLE_STATE, 0UL, &response) == APOLLO_STATUS_OK &&
			response == SD_R1_IDLE_STATE) {
			entered_idle = 1U;
			break;
		}
	}

	if (entered_idle == 0U) {
		return APOLLO_STATUS_TIMEOUT;
	}

	if (sd_send_command(card, SD_CMD8_SEND_IF_COND, SD_CMD8_ARGUMENT, &response) != APOLLO_STATUS_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if ((response & SD_R1_ILLEGAL_COMMAND) == 0U) {
		/* v2 card: the echo-back must match the pattern we sent. */
		if (sd_read_r3_r7(card, &if_cond) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		if ((if_cond & SD_CMD8_ECHO_MASK) != (SD_CMD8_ARGUMENT & SD_CMD8_ECHO_MASK)) {
			return APOLLO_STATUS_UNSUPPORTED;
		}
		is_v2 = 1U;
	}

	start = HAL_GetTick();
	do {
		uint32_t argument = (is_v2 != 0U) ? SD_ACMD41_HCS_ARG : 0UL;

		if (sd_send_app_command(card, SD_ACMD41_SEND_OP_COND, argument, &response) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		if (response == 0U) {
			break;
		}
	} while ((HAL_GetTick() - start) < SD_INIT_TIMEOUT_MS);

	if (response != 0U) {
		/* No SD response. Try MMC, which uses CMD1 rather than ACMD41. */
		start = HAL_GetTick();
		do {
			if (sd_send_command(card, 1U, 0UL, &response) != APOLLO_STATUS_OK) {
				return APOLLO_STATUS_HAL_ERROR;
			}
			if (response == 0U) {
				break;
			}
		} while ((HAL_GetTick() - start) < SD_INIT_TIMEOUT_MS);

		if (response != 0U) {
			return APOLLO_STATUS_TIMEOUT;
		}
		card->type = SD_CARD_TYPE_MMC;
	} else if (is_v2 != 0U) {
		if (sd_send_command(card, SD_CMD58_READ_OCR, 0UL, &response) != APOLLO_STATUS_OK ||
			sd_read_r3_r7(card, &ocr) != APOLLO_STATUS_OK) {
			return APOLLO_STATUS_HAL_ERROR;
		}
		card->type = ((ocr & SD_OCR_CCS_MASK) != 0U) ? SD_CARD_TYPE_SDHC : SD_CARD_TYPE_SD_V2;
	} else {
		card->type = SD_CARD_TYPE_SD_V1;
	}

	/* Byte addressed cards need an explicit 512 byte block length. */
	if (sd_type_is_block_addressed(card->type) == 0U) {
		if (sd_send_command(card, SD_CMD16_SET_BLOCKLEN, SD_BLOCK_SIZE, &response) != APOLLO_STATUS_OK ||
			response != 0U) {
			return APOLLO_STATUS_HAL_ERROR;
		}
	}

	return APOLLO_STATUS_OK;
}

static apollo_status_t sd_read_capacity(sd_card_t *card) {
	uint8_t response = 0U;
	uint8_t csd[16];

	if (sd_send_command(card, SD_CMD9_SEND_CSD, 0UL, &response) != APOLLO_STATUS_OK ||
		response != 0U) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (sd_read_data_block(card, csd, (uint32_t)sizeof(csd)) != APOLLO_STATUS_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	card->sector_count = sd_csd_sector_count(csd);

	return (card->sector_count > 0U) ? APOLLO_STATUS_OK : APOLLO_STATUS_UNSUPPORTED;
}

apollo_status_t sd_card_init(sd_card_t *card,
							 SPI_HandleTypeDef *spi,
							 GPIO_TypeDef *cs_port,
							 uint16_t cs_pin,
							 uint32_t timeout_ms) {
	apollo_status_t status;
	uint32_t previous_prescaler;
	uint8_t padding = SD_DUMMY_BYTE;

	if (card == NULL || spi == NULL || cs_port == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	card->spi = spi;
	card->cs_port = cs_port;
	card->cs_pin = cs_pin;
	card->timeout_ms = timeout_ms;
	card->type = SD_CARD_TYPE_UNKNOWN;
	card->sector_count = 0U;
	card->present = 0U;

	previous_prescaler = sd_prescaler_get(spi);
	sd_prescaler_set(spi, SD_CARD_INIT_PRESCALER);

	/* Power-up clocks are delivered with the card deselected. */
	HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
	for (uint8_t index = 0U; index < SD_POWERUP_PADDING_BYTES; index++) {
		(void)HAL_SPI_Transmit(spi, &padding, 1U, timeout_ms);
	}

	sd_select(card);
	status = sd_run_init_sequence(card);
	if (status == APOLLO_STATUS_OK) {
		status = sd_read_capacity(card);
	}
	sd_deselect(card);

	if (status == APOLLO_STATUS_OK) {
		card->present = 1U;
		sd_prescaler_set(spi, SD_CARD_DATA_PRESCALER);
	}

	sd_prescaler_set(spi, previous_prescaler);

	return status;
}

void sd_card_deinit(sd_card_t *card) {
	if (card == NULL) {
		return;
	}

	card->present = 0U;
	card->type = SD_CARD_TYPE_UNKNOWN;
	card->sector_count = 0U;
}

apollo_status_t sd_card_status(const sd_card_t *card) {
	if (card == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	return (card->present != 0U) ? APOLLO_STATUS_OK : APOLLO_STATUS_UNSUPPORTED;
}

uint8_t sd_card_is_present(const sd_card_t *card) {
	return (card != NULL && card->present != 0U) ? 1U : 0U;
}

uint32_t sd_card_sector_count(const sd_card_t *card) {
	return (card != NULL) ? card->sector_count : 0U;
}

sd_card_type_t sd_card_get_type(const sd_card_t *card) {
	return (card != NULL) ? card->type : SD_CARD_TYPE_UNKNOWN;
}

apollo_status_t sd_card_read_blocks(sd_card_t *card,
									uint32_t block,
									uint8_t *buffer,
									uint32_t count) {
	apollo_status_t status = APOLLO_STATUS_OK;
	uint32_t previous_prescaler;
	uint8_t response = 0U;

	if (card == NULL || buffer == NULL || count == 0U) {
		return APOLLO_STATUS_INVALID_ARG;
	}
	if (card->present == 0U) {
		return APOLLO_STATUS_UNSUPPORTED;
	}
	if (card->sector_count != 0U && (block + count) > card->sector_count) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	previous_prescaler = sd_prescaler_get(card->spi);
	sd_prescaler_set(card->spi, SD_CARD_DATA_PRESCALER);
	sd_select(card);

	if (count == 1U) {
		if (sd_send_command(card, SD_CMD17_READ_SINGLE_BLOCK,
							sd_block_to_argument(card->type, block), &response) != APOLLO_STATUS_OK ||
			response != 0U) {
			status = APOLLO_STATUS_HAL_ERROR;
		} else {
			status = sd_read_data_block(card, buffer, SD_BLOCK_SIZE);
		}
	} else {
		if (sd_send_command(card, SD_CMD18_READ_MULTI_BLOCK,
							sd_block_to_argument(card->type, block), &response) != APOLLO_STATUS_OK ||
			response != 0U) {
			status = APOLLO_STATUS_HAL_ERROR;
		} else {
			for (uint32_t index = 0U; index < count; index++) {
				status = sd_read_data_block(card, &buffer[index * SD_BLOCK_SIZE], SD_BLOCK_SIZE);
				if (status != APOLLO_STATUS_OK) {
					break;
				}
			}
			(void)sd_send_command(card, SD_CMD12_STOP_TRANSMISSION, 0UL, NULL);
			(void)sd_wait_not_busy(card, SD_BUSY_TIMEOUT_MS);
		}
	}

	sd_deselect(card);
	sd_prescaler_set(card->spi, previous_prescaler);

	if (status != APOLLO_STATUS_OK) {
		card->read_errors++;
	}

	return status;
}

apollo_status_t sd_card_write_blocks(sd_card_t *card,
									 uint32_t block,
									 const uint8_t *buffer,
									 uint32_t count) {
	apollo_status_t status = APOLLO_STATUS_OK;
	uint32_t previous_prescaler;
	uint8_t response = 0U;

	if (card == NULL || buffer == NULL || count == 0U) {
		return APOLLO_STATUS_INVALID_ARG;
	}
	if (card->present == 0U) {
		return APOLLO_STATUS_UNSUPPORTED;
	}
	if (card->sector_count != 0U && (block + count) > card->sector_count) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	previous_prescaler = sd_prescaler_get(card->spi);
	sd_prescaler_set(card->spi, SD_CARD_DATA_PRESCALER);
	sd_select(card);

	if (count == 1U) {
		if (sd_send_command(card, SD_CMD24_WRITE_BLOCK,
							sd_block_to_argument(card->type, block), &response) != APOLLO_STATUS_OK ||
			response != 0U) {
			status = APOLLO_STATUS_HAL_ERROR;
		} else {
			status = sd_write_data_block(card, SD_TOKEN_START_BLOCK, buffer, SD_BLOCK_SIZE);
		}
	} else {
		/* Telling an SD card how many blocks are coming lets it pre-erase and
		   avoids a stall between blocks. MMC has no equivalent. */
		if (card->type != SD_CARD_TYPE_MMC) {
			(void)sd_send_app_command(card, SD_ACMD23_SET_WR_BLK_COUNT, count, NULL);
		}

		if (sd_send_command(card, SD_CMD25_WRITE_MULTI_BLOCK,
							sd_block_to_argument(card->type, block), &response) != APOLLO_STATUS_OK ||
			response != 0U) {
			status = APOLLO_STATUS_HAL_ERROR;
		} else {
			for (uint32_t index = 0U; index < count; index++) {
				status = sd_write_data_block(card, SD_TOKEN_START_MULTI_WRITE,
											 &buffer[index * SD_BLOCK_SIZE], SD_BLOCK_SIZE);
				if (status != APOLLO_STATUS_OK) {
					break;
				}
			}

			if (status == APOLLO_STATUS_OK) {
				status = sd_write_data_block(card, SD_TOKEN_STOP_MULTI_WRITE, NULL, 0U);
			}
		}
	}

	sd_deselect(card);
	sd_prescaler_set(card->spi, previous_prescaler);

	if (status != APOLLO_STATUS_OK) {
		card->write_errors++;
	}

	return status;
}

apollo_status_t sd_card_sync(sd_card_t *card) {
	apollo_status_t status;
	uint32_t previous_prescaler;

	if (card == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}
	if (card->present == 0U) {
		return APOLLO_STATUS_UNSUPPORTED;
	}

	previous_prescaler = sd_prescaler_get(card->spi);
	sd_prescaler_set(card->spi, SD_CARD_DATA_PRESCALER);
	sd_select(card);
	status = sd_wait_not_busy(card, SD_BUSY_TIMEOUT_MS);
	sd_deselect(card);
	sd_prescaler_set(card->spi, previous_prescaler);

	return status;
}
