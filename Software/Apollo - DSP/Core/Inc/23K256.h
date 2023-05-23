/*
 * 23K256.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#ifndef INC_23K256_H_
#define INC_23K256_H_

#define PAGENUM	 	1024
#define PAGESIZE	32
#define I_READ		3
#define I_WRITE		2
#define WRSR		1
#define RFSR		5

#define MEM_TEST_V1 0xAA
#define MEM_TEST_V2 0x55

void testRAM(SPI_HandleTypeDef *spi);

void writeByteRAM(SPI_HandleTypeDef *spi, uint16_t address, uint8_t data);
void writePageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages, uint8_t data);

uint8_t readByteRAM(SPI_HandleTypeDef *spi, uint16_t address);
void readPageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages);


#endif /* INC_23K256_H_ */
