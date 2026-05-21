/*
 * 23K256.c
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */
#include "23K256.h"

static void ram_select(void) {
	HAL_GPIO_WritePin(RAM_CS_GPIO_PORT, RAM_CS_PIN, GPIO_PIN_RESET);
}

static void ram_deselect(void) {
	HAL_GPIO_WritePin(RAM_CS_GPIO_PORT, RAM_CS_PIN, GPIO_PIN_SET);
}

static HAL_StatusTypeDef ram_transmit(SPI_HandleTypeDef *spi, uint8_t *data, uint16_t size) {
	return HAL_SPI_Transmit(spi, data, size, RAM_TIMEOUT_MS);
}

static HAL_StatusTypeDef ram_transmit_command(SPI_HandleTypeDef *spi, uint8_t command) {
	return ram_transmit(spi, &command, 1U);
}

static HAL_StatusTypeDef ram_transmit_address(SPI_HandleTypeDef *spi, uint16_t address) {
	uint8_t address_bytes[2] = {
		(uint8_t)(address >> 8),
		(uint8_t)(address & 0xFFU)
	};

	return ram_transmit(spi, address_bytes, sizeof(address_bytes));
}

void set_RAM_Mode(SPI_HandleTypeDef *spi, uint8_t MODE) {
	ram_select();
	(void)ram_transmit_command(spi, WRSR);
	(void)ram_transmit(spi, &MODE, 1U);
	ram_deselect();
}

uint8_t testRAM(SPI_HandleTypeDef *spi) {
	uint8_t patterns[] = { MEM_TEST_V1, MEM_TEST_V2 };

	set_RAM_Mode(spi, SEQUENTIAL_MODE);

	for (uint32_t pattern_index = 0U; pattern_index < sizeof(patterns); pattern_index++) {
		uint8_t pattern = patterns[pattern_index];

		ram_select();
		if (ram_transmit_command(spi, I_WRITE) != HAL_OK ||
			ram_transmit_address(spi, 0U) != HAL_OK) {
			ram_deselect();
			return RAM_ERROR;
		}

		for (uint32_t i = 0U; i < RAM_SIZE_BYTES; i++) {
			if (ram_transmit(spi, &pattern, 1U) != HAL_OK) {
				ram_deselect();
				return RAM_ERROR;
			}
		}
		ram_deselect();

		ram_select();
		if (ram_transmit_command(spi, I_READ) != HAL_OK ||
			ram_transmit_address(spi, 0U) != HAL_OK) {
			ram_deselect();
			return RAM_ERROR;
		}

		for (uint32_t i = 0U; i < RAM_SIZE_BYTES; i++) {
			uint8_t rx_data = 0U;

			if (HAL_SPI_Receive(spi, &rx_data, 1U, RAM_TIMEOUT_MS) != HAL_OK ||
				rx_data != pattern) {
				ram_deselect();
				return RAM_ERROR;
			}
		}
		ram_deselect();
	}

	return RAM_OK;
}

void writeByteRAM(SPI_HandleTypeDef *spi, uint16_t address, uint8_t data) {
	ram_select();
	(void)ram_transmit_command(spi, I_WRITE);
	(void)ram_transmit_address(spi, address);
	(void)ram_transmit(spi, &data, 1U);
	ram_deselect();
}

uint8_t readByteRAM(SPI_HandleTypeDef *spi, uint16_t address) {
	uint8_t data = 0U;

	ram_select();
	if (ram_transmit_command(spi, I_READ) == HAL_OK &&
		ram_transmit_address(spi, address) == HAL_OK) {
		(void)HAL_SPI_Receive(spi, &data, 1U, RAM_TIMEOUT_MS);
	}
	ram_deselect();

	return data;
}

void writePageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages, uint8_t data) {
	uint32_t bytes_to_write = (uint32_t)pages * PAGESIZE;

	set_RAM_Mode(spi, SEQUENTIAL_MODE);

	ram_select();
	if (ram_transmit_command(spi, I_WRITE) == HAL_OK &&
		ram_transmit_address(spi, address) == HAL_OK) {
		for (uint32_t i = 0U; i < bytes_to_write; i++) {
			if (ram_transmit(spi, &data, 1U) != HAL_OK) {
				break;
			}
		}
	}
	ram_deselect();
}

void readPageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages) {
	uint32_t bytes_to_read = (uint32_t)pages * PAGESIZE;

	set_RAM_Mode(spi, SEQUENTIAL_MODE);

	ram_select();
	if (ram_transmit_command(spi, I_READ) == HAL_OK &&
		ram_transmit_address(spi, address) == HAL_OK) {
		for (uint32_t i = 0U; i < bytes_to_read; i++) {
			uint8_t data = 0U;
			if (HAL_SPI_Receive(spi, &data, 1U, RAM_TIMEOUT_MS) != HAL_OK) {
				break;
			}
		}
	}
	ram_deselect();
}
