/*
 * MCP4726.c
 *
 *  Created on: 28 mag 2023
 *      Author: Perri
 */

#include "MCP4726.h"

void initDAC(I2C_HandleTypeDef *i2c) {
	writeDAC(i2c, 0U);
}

void writeDAC(I2C_HandleTypeDef *i2c, uint16_t data) {
	uint16_t dac_value = data;
	uint8_t tx_data[2];

	if (dac_value > DAC_MAX_VALUE) {
		dac_value = DAC_MAX_VALUE;
	}

	tx_data[0] = (uint8_t)(DAC_REG_ADDRESS | ((dac_value >> 8) & 0x0FU));
	tx_data[1] = (uint8_t)(dac_value & 0xFFU);

	(void)HAL_I2C_Master_Transmit(i2c, DAC_I2C_ADDRESS, tx_data, sizeof(tx_data), DAC_TIMEOUT_MS);
}
