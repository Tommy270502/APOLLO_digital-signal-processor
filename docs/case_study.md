# Case Study: APOLLO Digital Signal Processor

APOLLO demonstrates the full path from custom embedded hardware design to
structured firmware and validation planning. The board combines analog
front-end circuitry, an STM32F401 microcontroller, USB CDC communication, an
external I2C DAC, SPI SRAM, and a microSD hardware footprint.

The firmware refactor focuses on maintainability and honesty. Generated CubeMX
code remains isolated, application behavior is moved into small modules, and
hardware-dependent limitations are called out directly. The result is a public
repository that shows embedded firmware architecture, board-level driver design,
DSP implementation, documentation discipline, and practical validation thinking.

## Design Tradeoffs

- Polling ADC acquisition is retained for this cleanup pass to avoid unverified
  timing changes. The code now isolates acquisition so DMA can be introduced
  later.
- The DSP filters use fixed buffers instead of dynamic allocation to keep memory
  use deterministic.
- microSD support is reported as unsupported rather than represented by no-op
  placeholder functions.
- The DAC driver is named for the board-level function instead of a specific
  MCP472x variant because the schematic uses an MCP4725-family symbol.

## Client-Relevant Signals

- Ability to clean and structure a mixed hardware/firmware repository.
- Practical STM32 HAL and CubeMX boundary management.
- Embedded C design with bounded buffers and explicit status codes.
- Validation planning that distinguishes host tests from hardware tests.
