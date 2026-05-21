/*
 * 23K256.h
 *
 *  Created on: 23 mag 2023
 *      Author: Perri
 */

#ifndef INC_23K256_H_
#define INC_23K256_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define PAGENUM             1024U
#define PAGESIZE            32U
#define RAM_SIZE_BYTES      (PAGENUM * PAGESIZE)

#define I_READ              0x03U
#define I_WRITE             0x02U
#define WRSR                0x01U
#define RFSR                0x05U

#define BYTE_MODE           0x00U
#define SEQUENTIAL_MODE     0x40U
#define PAGE_MODE           0x80U

#define MEM_TEST_V1 0xAA
#define MEM_TEST_V2 0x55

#define RAM_OK              0U
#define RAM_ERROR           1U
#define RAM_TIMEOUT_MS      100U

#define RAM_CS_GPIO_PORT    GPIOA
#define RAM_CS_PIN          GPIO_PIN_2

void set_RAM_Mode(SPI_HandleTypeDef *spi, uint8_t MODE);
uint8_t testRAM(SPI_HandleTypeDef *spi);

void writeByteRAM(SPI_HandleTypeDef *spi, uint16_t address, uint8_t data);
void writePageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages, uint8_t data);

uint8_t readByteRAM(SPI_HandleTypeDef *spi, uint16_t address);
void readPageRAM(SPI_HandleTypeDef *spi, uint16_t address, uint16_t pages);


#endif /* INC_23K256_H_ */
