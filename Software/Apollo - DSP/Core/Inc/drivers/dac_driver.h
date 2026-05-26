#ifndef DRIVERS_DAC_DRIVER_H_
#define DRIVERS_DAC_DRIVER_H_

#include <stdint.h>
#include "apollo_status.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAC_DRIVER_DEFAULT_ADDRESS_7BIT    0x60U
#define DAC_DRIVER_MAX_CODE                4095U

typedef struct {
	I2C_HandleTypeDef *i2c;
	uint16_t address;
	uint32_t timeout_ms;
} dac_driver_t;

apollo_status_t dac_driver_init(dac_driver_t *driver,
								I2C_HandleTypeDef *i2c,
								uint8_t address_7bit,
								uint32_t timeout_ms);
apollo_status_t dac_driver_write(dac_driver_t *driver, uint16_t code);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_DAC_DRIVER_H_ */
