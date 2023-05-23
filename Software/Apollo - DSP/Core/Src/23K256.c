/*
 * 23K256.c
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */
#include "23K256.h"



void testRAM(SPI_HandleTypeDef *spi) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(spi, WRSR, sizeof(WRSR), 100);
	HAL_SPI_Transmit(spi, WRSR, sizeof(WRSR), 100);
	for(uint16_t i=0;i<32767; i++){
			HAL_SPI_Transmit(spi, I_WRITE, sizeof(I_WRITE), 100);
			HAL_SPI_Transmit(spi, i, sizeof(i), 100);
			HAL_SPI_Transmit(spi, MEM_TEST_V1, sizeof(MEM_TEST_V1), 100);
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

void writeByteRAM(SPI_HandleTypeDef *spi, uint16_t address, uint8_t data) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(spi, I_WRITE, sizeof(I_WRITE), 100);
	HAL_SPI_Transmit(spi, address, sizeof(address), 100);
	HAL_SPI_Transmit(spi, data, sizeof(data), 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

void writePageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages, uint8_t data) {

}

uint8_t readByteRAM(SPI_HandleTypeDef *spi, uint16_t address) {
	uint8_t data;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(spi, I_READ, sizeof(I_READ), 100);
	HAL_SPI_Transmit(spi, address, sizeof(address), 100);
	HAL_SPI_Receive(spi, data, sizeof(data), 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
	return data;
}
void readPageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages);

