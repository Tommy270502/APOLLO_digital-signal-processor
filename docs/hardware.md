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

The microSD socket is present in hardware. Firmware support is intentionally
reported as unsupported until an SPI block-device layer, card initialization,
and filesystem integration are implemented and validated.
