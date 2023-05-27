/*
 * 23K256.c
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */
#include "23K256.h"


void set_RAM_Mode(SPI_HandleTypeDef *spi, uint8_t MODE) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&spi, WRSR, sizeof(WRSR), 100);
	HAL_SPI_Transmit(&spi, MODE, sizeof(MODE), 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

uint8_t testRAM(SPI_HandleTypeDef *spi) {
	uint8_t RX_Data = 0;
	uint8_t err = 0;
	uint16_t i=0;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&spi, WRSR, sizeof(WRSR), 100);
	HAL_SPI_Transmit(&spi, SEQUENTIAL_MODE, sizeof(SEQUENTIAL_MODE), 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&spi, I_WRITE, sizeof(I_WRITE), 100);
	HAL_SPI_Transmit(spi, 0, sizeof(0), 100);
	for(i=0; i<32767; i++) {
		HAL_SPI_Transmit(&spi, MEM_TEST_V1, sizeof(MEM_TEST_V1), 100);
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&spi, I_WRITE, sizeof(I_WRITE), 100);
	HAL_SPI_Transmit(&spi, 0, sizeof(0), 100);
	for(i=0; i<32767; i++) {
		HAL_SPI_Transmit(&spi, MEM_TEST_V2, sizeof(MEM_TEST_V1), 100);
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&spi, I_READ, sizeof(I_READ), 100);
	HAL_SPI_Transmit(&spi, 0, sizeof(0), 100);
	for(i=0; i<32767; i++) {
		HAL_SPI_Receive(&spi, RX_Data, sizeof(RX_Data), 100);
		if(RX_Data == 0) {
			err = 1;
		}
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
	return err;
}

void writeByteRAM(SPI_HandleTypeDef *spi, uint16_t address, uint8_t data) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_SPI_Transmit(spi, I_WRITE, sizeof(I_WRITE), 100);
	HAL_SPI_Transmit(spi, address, sizeof(address), 100);
	HAL_SPI_Transmit(spi, data, sizeof(data), 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
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

