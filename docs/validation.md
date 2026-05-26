# Validation

## Host Test Results

| Date | Platform | Compiler | Result |
| --- | --- | --- | --- |
| 2026-05-27 | Windows 11 / MSYS2 | GCC 15.2.0 | All tests pass |

Run:

```sh
make -C tests/host test
```

Tests cover:
- DSP filters (lowpass, highpass, EMA, moving average, median)
- CLI command parsing (filter, input, demo, telemetry, calibrate)
- Calibration validation (gain bounds, offset bounds, signal-path math)
- Signal chain saturation and DAC clipping flags
- Diagnostics (sample counting, min/max/mean ADC)
- Telemetry formatting (sample and status frames)

## Firmware Build Results

| Date | Configuration | Compiler | Result | Size (text/data/bss) |
| --- | --- | --- | --- | --- |
| 2026-05-27 | Debug | arm-none-eabi-gcc 14.3.1 | 0 errors, 1 warning | 74,976 / 716 / 8,024 |
| 2026-05-27 | Release | arm-none-eabi-gcc 14.3.1 | 0 errors, 1 warning | 49,816 / 712 / 8,024 |

Built via STM32CubeIDE 2.0.0 headless builder. The single warning is a
standard bare-metal RWX LOAD segment note from the linker.

All new source directories (`Core/Src/app/`, `Core/Src/dsp/`,
`Core/Src/drivers/`, `Core/Src/platform/`) are discovered automatically
by the managed build via the `Core` source entry in `.cproject`.

## Board Smoke Validation (Phase 3)

Not yet performed — requires physical board.

- [ ] USB CDC enumeration
- [ ] `help` command response
- [ ] `status` command response
- [ ] Malformed command error handling
- [ ] `demo sine`, `demo step`, `demo impulse`
- [ ] `filter bypass`, `filter lowpass 100`, `filter highpass 100`
- [ ] `filter average 5`, `filter median 5`
- [ ] Telemetry frame format over USB CDC
- [ ] `input 0` and `input 1` ADC readings
- [ ] Known voltage readings on both channels
- [ ] DAC follows filtered/demo signal
- [ ] SRAM self-test status
- [ ] No increasing SRAM error count during normal logging

## Phase Status

| Phase | Status | Notes |
| --- | --- | --- |
| 1. Stabilize refactor | Done | Commits split and tracked |
| 2. Host tests / CI | Done | Tests pass on Windows; CI not yet run |
| 3. Board smoke | Partial | Firmware builds; hardware tests require physical board |
| 4. USB/app robustness | Done | RX overflow counter, startup reporting |
| 5. Calibration | Done | calibrate adc/dac commands with validation |
| 6. ADC timer/DMA | Blocked | Depends on Phase 3 board validation |
| 7. microSD | Unsupported | Documented as future work |
| 8. README | Partial | Updated after host test verification |

## Known Limits

- ADC uses bounded polling, not timer-triggered DMA.
- microSD firmware support is not implemented.
- Hardware behavior must be validated on the actual board.
- GitHub CI has not been triggered yet (not pushed to remote).
- Phase 3 hardware smoke and Phase 6 ADC DMA require physical board access.
