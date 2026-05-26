# Testing

Host-side tests live in `tests/host` and exercise logic that does not depend on
STM32 HAL or board hardware.

```sh
make -C tests/host test
```

Covered areas:

- low-pass step response
- high-pass step decay
- moving average circular window behavior
- median outlier rejection
- filter parameter validation
- USB command parsing
- telemetry formatting
- diagnostics mean/min/max accounting
- signal-chain DAC clipping flags

The firmware build remains an STM32CubeIDE workflow because generated build
directories are not tracked.
