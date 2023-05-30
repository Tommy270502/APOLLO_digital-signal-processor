/*
 * SDCARD.h
 *
 *  Created on: 30 mag 2023
 *      Author: Perri
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

#ifndef INC_SDCARD_H_
#define INC_SDCARD_H_

uint8_t initSDCARD();
uint8_t readByteSDCARD(SPI_HandleTypeDef *spi);
void writeByteSDCARD(SPI_HandleTypeDef *spi, uint8_t data);
void clearSDCARD(SPI_HandleTypeDef *spi);

#endif /* INC_SDCARD_H_ */
