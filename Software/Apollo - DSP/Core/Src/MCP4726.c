/*
 * MCP4726.c
 *
 *  Created on: 28 mag 2023
 *      Author: Perri
 */

#include "MCP4726.h"

void initDAC(I2C_HandleTypeDef *i2c) {

	HAL_I2C_Mem_Write(&i2c, DAC_I2C_ADDRESS, DAC_REG_ADDRESS, sizeof(DAC_REG_ADDRESS), 0, sizeof(0), 100);
}

void writeDAC(I2C_HandleTypeDef *i2c, uint16_t data) {

	HAL_I2C_Mem_Write(&i2c, DAC_I2C_ADDRESS, DAC_REG_ADDRESS | (data>>4), sizeof(DAC_REG_ADDRESS), data, sizeof(data), 100);
}
