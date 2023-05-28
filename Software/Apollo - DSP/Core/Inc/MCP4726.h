/*
 * MCP4726.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

#ifndef INC_MCP4726_H_
#define INC_MCP4726_H_

#define DAC_I2C_ADDRESS 	0b01100000
#define DAC_COMMAND_BITS	0b00000000
#define DAC_PWDWN_NORMAL	0b00000000
#define DAC_PWDWN_1K		0b00010000
#define DAC_PWDWN_125K		0b00100000
#define DAC_PWDWN_640K		0b00110000

void initDAC(I2C_HandleTypeDef *i2c);
void writeDAC(I2C_HandleTypeDef *i2c, uint16_t data);

#endif /* INC_MCP4726_H_ */
