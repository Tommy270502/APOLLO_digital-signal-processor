/*
 * AT25SF081B.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#ifndef INC_AT25SF081B_H_
#define INC_AT25SF081B_H_

void initFLASH(SPI_HandleTypeDef *spi);

void writeByteFLASH(uint16_t address,uint8_t data);
void writePageFLASH(uint16_t address, uint16_t pages, uint8_t* data, size_t length);

void readByteFLASH(uint16_t address, uint8_t data);
void readPageFLASH(uint16_t address, uint16_t pages);

#endif /* INC_AT25SF081B_H_ */
