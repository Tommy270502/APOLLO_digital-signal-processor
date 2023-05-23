/*
 * MCP4726.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#ifndef INC_MCP4726_H_
#define INC_MCP4726_H_

typedef struct {

}DAC;

void initDAC(I2C_HandleTypeDef *i2c);
void writeDAC(DAC *handle, I2C_HandleTypeDef *i2c);

#endif /* INC_MCP4726_H_ */
