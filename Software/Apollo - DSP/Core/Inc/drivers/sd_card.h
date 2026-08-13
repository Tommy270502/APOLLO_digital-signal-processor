#ifndef DRIVERS_SD_CARD_H_
#define DRIVERS_SD_CARD_H_

/*
 * microSD block device over SPI.
 *
 * The card shares SPI1 with the 23K256 SRAM. Two consequences are handled
 * here rather than by callers:
 *
 *  - SD cards must be clocked at 400 kHz or slower until initialisation
 *    completes, while the SRAM runs the bus at full speed. Every public
 *    entry point sets the prescaler it needs and restores the previous value
 *    before returning, so the SRAM driver is unaffected.
 *  - The card only releases MISO after an extra clock cycle following CS
 *    going high, so each transaction ends with one padding byte.
 */

#include <stdint.h>
#include "apollo_status.h"
#include "drivers/sd_card_proto.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Board wiring: SD_nCS on PB0, sharing SPI1 with the SRAM on PA2. */
#define SD_CARD_CS_GPIO_PORT        GPIOB
#define SD_CARD_CS_PIN              GPIO_PIN_0

/* 84 MHz APB2 / 256 = 328 kHz, inside the 400 kHz initialisation limit. */
#define SD_CARD_INIT_PRESCALER      SPI_BAUDRATEPRESCALER_256
/* 84 MHz APB2 / 4 = 21 MHz, inside the 25 MHz SPI mode ceiling. */
#define SD_CARD_DATA_PRESCALER      SPI_BAUDRATEPRESCALER_4

typedef struct {
	SPI_HandleTypeDef *spi;
	GPIO_TypeDef *cs_port;
	uint16_t cs_pin;
	uint32_t timeout_ms;

	sd_card_type_t type;
	uint32_t sector_count;

	uint8_t present;          /* card initialised and usable */
	uint32_t read_errors;
	uint32_t write_errors;
} sd_card_t;

/* Runs the SPI mode initialisation sequence. Returns APOLLO_STATUS_OK when a
   card is present and addressable, APOLLO_STATUS_TIMEOUT when no card
   responds, or APOLLO_STATUS_HAL_ERROR on a bus fault. Safe to call again to
   re-detect a card after removal. */
apollo_status_t sd_card_init(sd_card_t *card,
							 SPI_HandleTypeDef *spi,
							 GPIO_TypeDef *cs_port,
							 uint16_t cs_pin,
							 uint32_t timeout_ms);

/* Marks the card as absent without touching the bus. */
void sd_card_deinit(sd_card_t *card);

apollo_status_t sd_card_status(const sd_card_t *card);
uint8_t sd_card_is_present(const sd_card_t *card);
uint32_t sd_card_sector_count(const sd_card_t *card);
sd_card_type_t sd_card_get_type(const sd_card_t *card);

apollo_status_t sd_card_read_blocks(sd_card_t *card,
									uint32_t block,
									uint8_t *buffer,
									uint32_t count);

apollo_status_t sd_card_write_blocks(sd_card_t *card,
									 uint32_t block,
									 const uint8_t *buffer,
									 uint32_t count);

/* Waits for any in-flight programming to finish. */
apollo_status_t sd_card_sync(sd_card_t *card);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SD_CARD_H_ */
