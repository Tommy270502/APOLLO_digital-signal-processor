# Validation

## Host Test Results

| Date | Platform | Compiler | Result |
| --- | --- | --- | --- |
| 2026-05-27 | Windows 11 / MSYS2 | GCC 15.2.0 | All tests pass |

Run:

```sh
make -C tests/host test
```

This checks host-buildable DSP, command parsing, telemetry, diagnostics, and
signal-chain logic.

## Firmware Build Validation

1. Import `Software/Apollo - DSP/` into STM32CubeIDE.
2. Build Debug or Release.
3. Confirm no missing include paths for `Core/Inc/app`, `Core/Inc/dsp`,
   `Core/Inc/drivers`, or `Core/Inc/platform`.
4. Flash using SWD.

## Manual Hardware Validation

- Confirm USB CDC enumeration and command response with `help` and `status`.
- Run `demo sine` and inspect telemetry and DAC output.
- Switch `input 0` and `input 1` and verify ADC readings against known input
  voltages.
- Confirm SRAM self-test result in `status`; verify no SRAM error count during
  logging.
- Measure DAC output for low, mid-scale, and full-scale commands via demo or
  controlled ADC inputs.

## Known Limits

- ADC DMA/timer sampling is configured only partially in CubeMX and is not the
  active acquisition path.
- microSD firmware support is not implemented.
- Hardware behavior has to be validated on the actual board.
