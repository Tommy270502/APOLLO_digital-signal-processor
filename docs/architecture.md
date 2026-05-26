# Architecture

APOLLO separates generated STM32CubeIDE infrastructure from hand-written
application firmware. CubeMX owns peripheral setup in `main.c`, MSP files, USB
device glue, HAL, CMSIS, and middleware. Application code lives below
`Core/Inc/app`, `Core/Inc/dsp`, `Core/Inc/drivers`, and `Core/Inc/platform`
with matching source folders.

```mermaid
flowchart TD
    Main["CubeMX main.c"] --> App["apollo_app"]
    App --> Platform["platform\nADC + USB CDC helpers"]
    App --> Signal["signal chain"]
    Signal --> DSP["DSP filters"]
    App --> Drivers["DAC + SRAM + SD status drivers"]
    App --> Telemetry["telemetry formatter"]
    App --> Diagnostics["diagnostics counters"]
    App --> CLI["command parser"]
```

The main runtime path is tick scheduled. ADC polling is still used in this
cleanup pass, but it is bounded and isolated in `platform/apollo_adc.c` so a
future timer/DMA implementation can replace it without rewriting the signal
chain.

## Data Flow

```mermaid
flowchart LR
    ADC["ADC or demo source"] --> Process["calibrate + filter"]
    Process --> DAC["DAC code clamp"]
    DAC --> I2C["I2C DAC write"]
    Process --> USB["CSV telemetry"]
    DAC --> SRAM["8-byte SRAM record"]
    Process --> Diag["diagnostics"]
```

The SRAM record format is little-endian:

| Offset | Type | Field |
| --- | --- | --- |
| 0 | `uint32_t` | timestamp in milliseconds |
| 4 | `uint16_t` | raw ADC code |
| 6 | `uint16_t` | DAC code |
