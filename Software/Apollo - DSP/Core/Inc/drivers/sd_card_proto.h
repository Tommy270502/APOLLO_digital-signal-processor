#ifndef DRIVERS_SD_CARD_PROTO_H_
#define DRIVERS_SD_CARD_PROTO_H_

/*
 * SD SPI-mode protocol logic, with no HAL dependency.
 *
 * Everything here is pure computation on buffers, so the host test suite can
 * cover it directly. Transport lives in sd_card.c.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Command indices used by this driver */
#define SD_CMD0_GO_IDLE_STATE       0U
#define SD_CMD8_SEND_IF_COND        8U
#define SD_CMD9_SEND_CSD            9U
#define SD_CMD12_STOP_TRANSMISSION  12U
#define SD_CMD16_SET_BLOCKLEN       16U
#define SD_CMD17_READ_SINGLE_BLOCK  17U
#define SD_CMD18_READ_MULTI_BLOCK   18U
#define SD_CMD24_WRITE_BLOCK        24U
#define SD_CMD25_WRITE_MULTI_BLOCK  25U
#define SD_CMD55_APP_CMD            55U
#define SD_CMD58_READ_OCR           58U
#define SD_ACMD23_SET_WR_BLK_COUNT  23U
#define SD_ACMD41_SEND_OP_COND      41U

/* R1 response bits. 0x00 means ready; 0x01 means idle. A set MSB means the
   byte was not an R1 response at all. */
#define SD_R1_IDLE_STATE            0x01U
#define SD_R1_ILLEGAL_COMMAND       0x04U

/* Data tokens */
#define SD_TOKEN_START_BLOCK        0xFEU
#define SD_TOKEN_START_MULTI_WRITE  0xFCU
#define SD_TOKEN_STOP_MULTI_WRITE   0xFDU

/* Data response mask/values returned after a write block */
#define SD_DATA_RESPONSE_MASK       0x1FU
#define SD_DATA_RESPONSE_ACCEPTED   0x05U

#define SD_BLOCK_SIZE               512U
#define SD_COMMAND_FRAME_BYTES      6U

/* OCR bit 30 — card capacity status. Set means block addressing (SDHC/SDXC). */
#define SD_OCR_CCS_MASK             0x40000000UL

/* ACMD41 argument bit 30 — host capacity support */
#define SD_ACMD41_HCS_ARG           0x40000000UL

/* CMD8 check pattern: 2.7-3.6V supply, pattern 0xAA */
#define SD_CMD8_ARGUMENT            0x000001AAUL
#define SD_CMD8_ECHO_MASK           0x00000FFFUL

typedef enum {
	SD_CARD_TYPE_UNKNOWN = 0,
	SD_CARD_TYPE_MMC,        /* MMC v3, byte addressed */
	SD_CARD_TYPE_SD_V1,      /* SD v1.x, byte addressed */
	SD_CARD_TYPE_SD_V2,      /* SD v2 standard capacity, byte addressed */
	SD_CARD_TYPE_SDHC        /* SD v2 high/extended capacity, block addressed */
} sd_card_type_t;

/* CRC7 with the trailing stop bit already applied, ready to send as the sixth
   command byte. */
uint8_t sd_crc7(const uint8_t *data, size_t length);

/* CRC16-CCITT over a data block, as used by the card's data tokens. */
uint16_t sd_crc16(const uint8_t *data, size_t length);

/* Fills a six byte command frame: start bits, index, argument, CRC7. */
void sd_build_command(uint8_t *frame, uint8_t command, uint32_t argument);

/* Sector count derived from a 16 byte CSD register. Handles both CSD v1
   (capacity encoded via C_SIZE/C_SIZE_MULT/READ_BL_LEN) and CSD v2 (a plain
   C_SIZE in 512 KiB units). Returns 0 if the structure version is unknown. */
uint32_t sd_csd_sector_count(const uint8_t *csd);

/* True when a card of this type addresses media by block rather than byte. */
uint8_t sd_type_is_block_addressed(sd_card_type_t type);

/* Converts a logical block address into the argument the card expects,
   accounting for byte addressed cards. */
uint32_t sd_block_to_argument(sd_card_type_t type, uint32_t block);

const char *sd_card_type_name(sd_card_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SD_CARD_PROTO_H_ */
