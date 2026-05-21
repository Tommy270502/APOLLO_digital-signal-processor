/*
 * SDCARD.c
 *
 *  Created on: 30 mag 2023
 *      Author: Perri
 */

#include "SDCARD.h"

uint8_t initSDCARD() {
	return 1U;
}

uint8_t readByteSDCARD(SPI_HandleTypeDef *spi) {
	(void)spi;

	return 0U;
}

void writeByteSDCARD(SPI_HandleTypeDef *spi, uint8_t data) {
	(void)spi;
	(void)data;

}

void clearSDCARD(SPI_HandleTypeDef *spi) {
	(void)spi;

}
