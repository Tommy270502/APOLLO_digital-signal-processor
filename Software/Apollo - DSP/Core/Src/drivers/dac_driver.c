#include "drivers/dac_driver.h"

#include <stddef.h>

apollo_status_t dac_driver_init(dac_driver_t *driver,
								I2C_HandleTypeDef *i2c,
								uint8_t address_7bit,
								uint32_t timeout_ms) {
	if (driver == NULL || i2c == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	driver->i2c = i2c;
	driver->address = (uint16_t)(address_7bit << 1U);
	driver->timeout_ms = timeout_ms;

	return dac_driver_write(driver, 0U);
}

apollo_status_t dac_driver_write(dac_driver_t *driver, uint16_t code) {
	uint16_t dac_code;
	uint8_t tx_data[2];

	if (driver == NULL || driver->i2c == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	dac_code = code;
	if (dac_code > DAC_DRIVER_MAX_CODE) {
		dac_code = DAC_DRIVER_MAX_CODE;
	}

	/* MCP4725-family fast-mode 12-bit write: command/power bits plus D11..D8, then D7..D0. */
	tx_data[0] = (uint8_t)((dac_code >> 8) & 0x0FU);
	tx_data[1] = (uint8_t)(dac_code & 0xFFU);

	if (HAL_I2C_Master_Transmit(driver->i2c,
								driver->address,
								tx_data,
								(uint16_t)sizeof(tx_data),
								driver->timeout_ms) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}
