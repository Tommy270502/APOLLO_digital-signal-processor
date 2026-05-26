#include "drivers/sram_23k256.h"

#include <stddef.h>

#define SRAM_CMD_READ        0x03U
#define SRAM_CMD_WRITE       0x02U
#define SRAM_CMD_WRSR        0x01U

#define SRAM_TEST_V1         0xAAU
#define SRAM_TEST_V2         0x55U

static void sram_select(sram_23k256_t *driver) {
	HAL_GPIO_WritePin(driver->cs_port, driver->cs_pin, GPIO_PIN_RESET);
}

static void sram_deselect(sram_23k256_t *driver) {
	HAL_GPIO_WritePin(driver->cs_port, driver->cs_pin, GPIO_PIN_SET);
}

static apollo_status_t sram_transmit(sram_23k256_t *driver, const uint8_t *data, uint16_t length) {
	if (HAL_SPI_Transmit(driver->spi, (uint8_t *)data, length, driver->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}

static apollo_status_t sram_receive(sram_23k256_t *driver, uint8_t *data, uint16_t length) {
	if (HAL_SPI_Receive(driver->spi, data, length, driver->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}

static apollo_status_t sram_transmit_command(sram_23k256_t *driver, uint8_t command) {
	return sram_transmit(driver, &command, 1U);
}

static apollo_status_t sram_transmit_address(sram_23k256_t *driver, uint16_t address) {
	uint8_t address_bytes[2] = {
		(uint8_t)(address >> 8),
		(uint8_t)(address & 0xFFU)
	};

	return sram_transmit(driver, address_bytes, (uint16_t)sizeof(address_bytes));
}

apollo_status_t sram_23k256_init(sram_23k256_t *driver,
								 SPI_HandleTypeDef *spi,
								 GPIO_TypeDef *cs_port,
								 uint16_t cs_pin,
								 uint32_t timeout_ms) {
	if (driver == NULL || spi == NULL || cs_port == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	driver->spi = spi;
	driver->cs_port = cs_port;
	driver->cs_pin = cs_pin;
	driver->timeout_ms = timeout_ms;
	sram_deselect(driver);

	return sram_23k256_set_mode(driver, SRAM_23K256_MODE_BYTE);
}

apollo_status_t sram_23k256_set_mode(sram_23k256_t *driver, sram_23k256_mode_t mode) {
	apollo_status_t status = APOLLO_STATUS_OK;
	uint8_t mode_byte = (uint8_t)mode;

	if (driver == NULL || driver->spi == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	sram_select(driver);
	if (sram_transmit_command(driver, SRAM_CMD_WRSR) != APOLLO_STATUS_OK ||
		sram_transmit(driver, &mode_byte, 1U) != APOLLO_STATUS_OK) {
		status = APOLLO_STATUS_HAL_ERROR;
	}
	sram_deselect(driver);

	return status;
}

apollo_status_t sram_23k256_write(sram_23k256_t *driver,
								  uint16_t address,
								  const uint8_t *data,
								  uint16_t length) {
	apollo_status_t status = APOLLO_STATUS_OK;

	if (driver == NULL || driver->spi == NULL || data == NULL || length == 0U) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if ((uint32_t)address + (uint32_t)length > SRAM_23K256_SIZE_BYTES) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	sram_select(driver);
	if (sram_transmit_command(driver, SRAM_CMD_WRITE) != APOLLO_STATUS_OK ||
		sram_transmit_address(driver, address) != APOLLO_STATUS_OK ||
		sram_transmit(driver, data, length) != APOLLO_STATUS_OK) {
		status = APOLLO_STATUS_HAL_ERROR;
	}
	sram_deselect(driver);

	return status;
}

apollo_status_t sram_23k256_read(sram_23k256_t *driver,
								 uint16_t address,
								 uint8_t *data,
								 uint16_t length) {
	apollo_status_t status = APOLLO_STATUS_OK;

	if (driver == NULL || driver->spi == NULL || data == NULL || length == 0U) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if ((uint32_t)address + (uint32_t)length > SRAM_23K256_SIZE_BYTES) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	sram_select(driver);
	if (sram_transmit_command(driver, SRAM_CMD_READ) != APOLLO_STATUS_OK ||
		sram_transmit_address(driver, address) != APOLLO_STATUS_OK ||
		sram_receive(driver, data, length) != APOLLO_STATUS_OK) {
		status = APOLLO_STATUS_HAL_ERROR;
	}
	sram_deselect(driver);

	return status;
}

apollo_status_t sram_23k256_write_byte(sram_23k256_t *driver, uint16_t address, uint8_t data) {
	return sram_23k256_write(driver, address, &data, 1U);
}

apollo_status_t sram_23k256_read_byte(sram_23k256_t *driver, uint16_t address, uint8_t *data) {
	return sram_23k256_read(driver, address, data, 1U);
}

apollo_status_t sram_23k256_test(sram_23k256_t *driver) {
	const uint8_t patterns[] = { SRAM_TEST_V1, SRAM_TEST_V2 };
	uint8_t rx_data = 0U;
	apollo_status_t status;

	if (driver == NULL || driver->spi == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	status = sram_23k256_set_mode(driver, SRAM_23K256_MODE_SEQUENTIAL);
	if (status != APOLLO_STATUS_OK) {
		return status;
	}

	for (uint8_t pattern_index = 0U; pattern_index < (uint8_t)sizeof(patterns); pattern_index++) {
		uint8_t pattern = patterns[pattern_index];

		sram_select(driver);
		if (sram_transmit_command(driver, SRAM_CMD_WRITE) != APOLLO_STATUS_OK ||
			sram_transmit_address(driver, 0U) != APOLLO_STATUS_OK) {
			sram_deselect(driver);
			return APOLLO_STATUS_HAL_ERROR;
		}

		for (uint32_t i = 0U; i < SRAM_23K256_SIZE_BYTES; i++) {
			if (sram_transmit(driver, &pattern, 1U) != APOLLO_STATUS_OK) {
				sram_deselect(driver);
				return APOLLO_STATUS_HAL_ERROR;
			}
		}
		sram_deselect(driver);

		sram_select(driver);
		if (sram_transmit_command(driver, SRAM_CMD_READ) != APOLLO_STATUS_OK ||
			sram_transmit_address(driver, 0U) != APOLLO_STATUS_OK) {
			sram_deselect(driver);
			return APOLLO_STATUS_HAL_ERROR;
		}

		for (uint32_t i = 0U; i < SRAM_23K256_SIZE_BYTES; i++) {
			if (sram_receive(driver, &rx_data, 1U) != APOLLO_STATUS_OK ||
				rx_data != pattern) {
				sram_deselect(driver);
				return APOLLO_STATUS_HAL_ERROR;
			}
		}
		sram_deselect(driver);
	}

	(void)sram_23k256_set_mode(driver, SRAM_23K256_MODE_BYTE);
	return APOLLO_STATUS_OK;
}
