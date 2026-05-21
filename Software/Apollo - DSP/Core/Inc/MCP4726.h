/*
 * MCP4726.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#ifndef INC_MCP4726_H_
#define INC_MCP4726_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define DAC_I2C_ADDRESS_7BIT    0x60U
#define DAC_I2C_ADDRESS         (DAC_I2C_ADDRESS_7BIT << 1U)

#define DAC_COMMAND_BITS        0x00U
#define DAC_PWDWN_NORMAL        0x00U
#define DAC_PWDWN_1K            0x10U
#define DAC_PWDWN_125K          0x20U
#define DAC_PWDWN_640K          0x30U
#define DAC_REG_ADDRESS         (DAC_COMMAND_BITS | DAC_PWDWN_NORMAL)

#define DAC_MAX_VALUE           4095U
#define DAC_TIMEOUT_MS          100U

void initDAC(I2C_HandleTypeDef *i2c);
void writeDAC(I2C_HandleTypeDef *i2c, uint16_t data);

#endif /* INC_MCP4726_H_ */
