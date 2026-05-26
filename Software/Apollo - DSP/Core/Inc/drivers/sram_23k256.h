#ifndef DRIVERS_SRAM_23K256_H_
#define DRIVERS_SRAM_23K256_H_

#include <stdint.h>
#include "apollo_status.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SRAM_23K256_PAGE_COUNT          1024U
#define SRAM_23K256_PAGE_SIZE_BYTES     32U
#define SRAM_23K256_SIZE_BYTES          (SRAM_23K256_PAGE_COUNT * SRAM_23K256_PAGE_SIZE_BYTES)

#define SRAM_23K256_CS_GPIO_PORT        GPIOA
#define SRAM_23K256_CS_PIN              GPIO_PIN_2

typedef enum {
	SRAM_23K256_MODE_BYTE = 0x00U,
	SRAM_23K256_MODE_SEQUENTIAL = 0x40U,
	SRAM_23K256_MODE_PAGE = 0x80U
} sram_23k256_mode_t;

typedef struct {
	SPI_HandleTypeDef *spi;
	GPIO_TypeDef *cs_port;
	uint16_t cs_pin;
	uint32_t timeout_ms;
} sram_23k256_t;

apollo_status_t sram_23k256_init(sram_23k256_t *driver,
								 SPI_HandleTypeDef *spi,
								 GPIO_TypeDef *cs_port,
								 uint16_t cs_pin,
								 uint32_t timeout_ms);
apollo_status_t sram_23k256_set_mode(sram_23k256_t *driver, sram_23k256_mode_t mode);
apollo_status_t sram_23k256_test(sram_23k256_t *driver);
apollo_status_t sram_23k256_write_byte(sram_23k256_t *driver, uint16_t address, uint8_t data);
apollo_status_t sram_23k256_read_byte(sram_23k256_t *driver, uint16_t address, uint8_t *data);
apollo_status_t sram_23k256_write(sram_23k256_t *driver,
								  uint16_t address,
								  const uint8_t *data,
								  uint16_t length);
apollo_status_t sram_23k256_read(sram_23k256_t *driver,
								 uint16_t address,
								 uint8_t *data,
								 uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SRAM_23K256_H_ */
