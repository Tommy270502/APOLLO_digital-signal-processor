/*
 * 23K256.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

#ifndef INC_23K256_H_
#define INC_23K256_H_

#define PAGENUM	 	1024
#define PAGESIZE	32
#define I_READ		3
#define I_WRITE		2
#define WRSR		1
#define RFSR		5

#define BYTE_MODE	0
#define SEQUENTIAL_MODE	64
#define PAGE_MODE	128

#define MEM_TEST_V1 0xAA
#define MEM_TEST_V2 0x55

void set_RAM_Mode(SPI_HandleTypeDef *spi, uint8_t MODE);
uint8_t testRAM(SPI_HandleTypeDef *spi);

void writeByteRAM(SPI_HandleTypeDef *spi, uint16_t address, uint8_t data);
void writePageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages, uint8_t data);

uint8_t readByteRAM(SPI_HandleTypeDef *spi, uint16_t address);
void readPageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages);


#endif /* INC_23K256_H_ */
