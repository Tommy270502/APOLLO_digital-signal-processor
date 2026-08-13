# Hardware

APOLLO is a USB-powered STM32F401RBTx board with analog input/output circuitry,
external SRAM, USB CDC connectivity, and debug headers.

## Main Blocks

- STM32F401RBTx MCU running from an HSE-based 84 MHz clock configuration.
- USB-C connector with USB OTG FS routed to PA11/PA12.
- Two analog input labels, `AIN0` and `AIN1`, routed through analog conditioning
  to ADC channel nets.
- MCP4725-family external I2C DAC on I2C1.
- 23K256 32 KiB SPI SRAM on SPI1.
- microSD socket sharing the SPI bus through a separate chip-select net.
- SWD and USART1 headers for programming, debug, and expansion.

## DAC Naming

The schematic uses an MCP4725-family KiCad symbol. The firmware now uses a
neutral `dac_driver` name and documents the driver as MCP4725-family compatible
instead of naming the source files after MCP4726.

## SD-Card Status

The microSD socket shares SPI1 with the SRAM through the separate `SD_nCS`
net on PB0. Firmware implements an SPI-mode block driver
(`drivers/sd_card.c`) under FatFs, logging samples as CSV.

Two constraints the driver handles rather than the caller:

- Cards must be clocked at 400 kHz or slower until initialisation completes,
  while the SRAM runs the bus at full speed. Each entry point sets the
  prescaler it needs and restores the previous value before returning.
- The card only releases MISO one clock after CS rises, so every transaction
  ends with a padding byte.

Protocol logic (CRC7, CRC16, CSD capacity decoding, command framing) lives in
`drivers/sd_card_proto.c`, which is HAL-free and covered by host tests
against the CMD0/CMD8 CRC values published in the SD specification.

Card behaviour has not been observed on hardware yet.
