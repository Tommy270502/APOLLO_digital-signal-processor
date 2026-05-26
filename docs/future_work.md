# Future Work

- Replace polling ADC acquisition with timer-triggered DMA and explicit overrun
  accounting.
- Add a real microSD SPI block-device layer and FatFs integration.
- Add calibration commands for ADC offset/gain and DAC scaling with persisted
  calibration data if nonvolatile storage is added.
- Add hardware-in-the-loop tests for ADC channel accuracy, DAC linearity, SRAM
  logging, and USB CDC throughput.
- Add firmware CI using a pinned STM32 toolchain container if reproducible build
  time and maintenance cost are acceptable.
