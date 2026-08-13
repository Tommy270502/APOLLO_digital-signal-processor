#include "drivers/sd_card_proto.h"

/* CRC7, polynomial x^7 + x^3 + 1. The card expects the result left-aligned in
   the byte with the stop bit in bit 0, which is what the final shift does. */
uint8_t sd_crc7(const uint8_t *data, size_t length) {
	uint8_t crc = 0U;

	if (data == NULL) {
		return 0x01U;
	}

	for (size_t index = 0U; index < length; index++) {
		uint8_t byte = data[index];

		for (uint8_t bit = 0U; bit < 8U; bit++) {
			/* The feedback test compares the message bit against the register
			   MSB *after* the shift; testing before it silently produces a
			   plausible but wrong CRC. */
			crc = (uint8_t)(crc << 1);
			if (((byte ^ crc) & 0x80U) != 0U) {
				crc ^= 0x09U;
			}
			byte = (uint8_t)(byte << 1);
		}
	}

	return (uint8_t)((crc << 1) | 0x01U);
}

/* CRC16-CCITT, polynomial 0x1021, zero seed. Used for data block tokens. */
uint16_t sd_crc16(const uint8_t *data, size_t length) {
	uint16_t crc = 0U;

	if (data == NULL) {
		return 0U;
	}

	for (size_t index = 0U; index < length; index++) {
		crc ^= (uint16_t)((uint16_t)data[index] << 8);

		for (uint8_t bit = 0U; bit < 8U; bit++) {
			if ((crc & 0x8000U) != 0U) {
				crc = (uint16_t)((uint16_t)(crc << 1) ^ 0x1021U);
			} else {
				crc = (uint16_t)(crc << 1);
			}
		}
	}

	return crc;
}

void sd_build_command(uint8_t *frame, uint8_t command, uint32_t argument) {
	if (frame == NULL) {
		return;
	}

	frame[0] = (uint8_t)(0x40U | (command & 0x3FU));
	frame[1] = (uint8_t)(argument >> 24);
	frame[2] = (uint8_t)(argument >> 16);
	frame[3] = (uint8_t)(argument >> 8);
	frame[4] = (uint8_t)(argument);
	frame[5] = sd_crc7(frame, 5U);
}

uint32_t sd_csd_sector_count(const uint8_t *csd) {
	uint8_t structure;

	if (csd == NULL) {
		return 0U;
	}

	structure = (uint8_t)((csd[0] >> 6) & 0x03U);

	if (structure == 1U) {
		/* CSD v2: C_SIZE counts 512 KiB units, so sectors = (C_SIZE + 1) * 1024. */
		uint32_t c_size = ((uint32_t)(csd[7] & 0x3FU) << 16) |
						  ((uint32_t)csd[8] << 8) |
						  (uint32_t)csd[9];

		return (c_size + 1U) * 1024U;
	}

	if (structure == 0U) {
		/* CSD v1: capacity = (C_SIZE + 1) * 2^(C_SIZE_MULT + 2) * 2^READ_BL_LEN. */
		uint32_t c_size = ((uint32_t)(csd[6] & 0x03U) << 10) |
						  ((uint32_t)csd[7] << 2) |
						  ((uint32_t)(csd[8] & 0xC0U) >> 6);
		uint8_t c_size_mult = (uint8_t)(((csd[9] & 0x03U) << 1) |
										((csd[10] & 0x80U) >> 7));
		uint8_t read_bl_len = (uint8_t)(csd[5] & 0x0FU);
		uint32_t block_count = (c_size + 1U) << (c_size_mult + 2U);

		/* Normalise whatever the card calls a block into 512 byte sectors. */
		if (read_bl_len >= 9U) {
			return block_count << (read_bl_len - 9U);
		}

		return block_count >> (9U - read_bl_len);
	}

	return 0U;
}

uint8_t sd_type_is_block_addressed(sd_card_type_t type) {
	return (type == SD_CARD_TYPE_SDHC) ? 1U : 0U;
}

uint32_t sd_block_to_argument(sd_card_type_t type, uint32_t block) {
	if (sd_type_is_block_addressed(type) != 0U) {
		return block;
	}

	return block * SD_BLOCK_SIZE;
}

const char *sd_card_type_name(sd_card_type_t type) {
	switch (type) {
	case SD_CARD_TYPE_MMC:
		return "mmc";
	case SD_CARD_TYPE_SD_V1:
		return "sdv1";
	case SD_CARD_TYPE_SD_V2:
		return "sdv2";
	case SD_CARD_TYPE_SDHC:
		return "sdhc";
	case SD_CARD_TYPE_UNKNOWN:
	default:
		return "unknown";
	}
}
