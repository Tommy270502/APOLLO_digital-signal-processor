# APOLLO Portfolio Refactor Plan

## Summary

This plan turns APOLLO from a functional project archive into a public embedded
engineering case study. The work is split into small phases so each change can
be reviewed independently and validated at the right level.

## Phase 1: Repository Hygiene

- Remove tracked generated artifacts and local state:
  - STM32CubeIDE workspace metadata in `Software/.metadata/`
  - Generated firmware build outputs in `Software/Apollo - DSP/Debug/` and
    `Software/Apollo - DSP/Release/`
  - KiCad lock/cache/local files and automatic backup ZIPs
  - Packaged ZIP snapshots
  - Broken or unsupported local artifacts such as the orphaned KiCad plugin
    gitlink and stale unused flash header
- Keep source assets needed to understand, build, fabricate, or validate the
  project:
  - KiCad schematics, PCB, project files, Gerbers, board pictures, datasheets
  - STM32CubeIDE project files, CubeMX `.ioc`, linker script, startup, HAL,
    CMSIS, USB middleware, and hand-written firmware source
- Strengthen `.gitignore` for STM32CubeIDE, KiCad, generated binaries,
  archives, temporary files, and local metadata.
- Add a short repository structure document.

## Phase 2: Firmware Architecture Cleanup

- Keep CubeMX generated code boundaries intact.
- Add clear hand-written firmware folders:
  - `Core/Inc/app` and `Core/Src/app`
  - `Core/Inc/dsp` and `Core/Src/dsp`
  - `Core/Inc/drivers` and `Core/Src/drivers`
  - `Core/Inc/platform` and `Core/Src/platform`
- Refactor `main.c` so it mainly performs HAL setup, CubeMX peripheral setup,
  application initialization, and the application run loop.
- Move application behavior into focused modules for configuration, signal
  processing, telemetry, diagnostics, storage, and platform access.

## Phase 3: DSP Feature Enhancement

- Replace the minimal filter module with a reusable embedded DSP filter library.
- Implement fixed-size, no-allocation filters:
  - bypass
  - first-order low-pass IIR
  - first-order high-pass IIR
  - exponential moving average
  - moving average
  - small-window median
- Validate parameters and expose reset/init/update APIs.
- Add a minimal command parser for runtime filter selection where practical.

## Phase 4: Signal-Chain Improvements

- Preserve the current default behavior while making the signal chain explicit.
- Add active ADC channel selection between the two wired analog inputs.
- Add calibration, clipping/saturation flags, diagnostics, and a synthetic demo
  source.
- Expand telemetry to include timestamp, sequence, channel, raw ADC, filtered
  value, DAC code, filter mode, and flags.
- Document SRAM circular logging format.

## Phase 5: ADC and Timing Review

- Isolate ADC acquisition behind a platform module.
- Replace unbounded ADC waits and delay-driven timing with bounded polling and
  tick-based scheduling in this cleanup pass.
- Document timer/DMA sampling as a future upgrade unless it is implemented and
  hardware-validated later.

## Phase 6: Driver Cleanup

- Improve external SRAM and DAC APIs with status returns and input validation.
- Rename or document the external DAC driver using neutral MCP4725-family
  naming.
- Keep USB CDC telemetry nonblocking where possible and account for busy states.
- Replace SD-card no-op stubs with an honest unsupported status.

## Phase 7: Tests and Host-Side Validation

- Add a lightweight host-buildable C test runner.
- Cover DSP filters, command parsing, telemetry formatting, calibration math,
  and circular-buffer indexing.
- Use plain `gcc` and `make` instead of a heavy test framework.

## Phase 8: Documentation

- Upgrade README into a concise public case study.
- Add focused technical docs:
  - architecture
  - firmware
  - DSP filters
  - demo usage
  - hardware
  - testing
  - validation
  - future work
- Use Mermaid diagrams for architecture, data flow, and state-machine views.

## Phase 9: CI and Automation

- Add GitHub Actions for host-side tests and lightweight documentation/repo
  sanity checks.
- Document manual STM32CubeIDE build and flash steps instead of pretending
  firmware CI exists if the embedded toolchain is not installed in CI.

## Phase 10: Portfolio Polish

- Add engineering highlights, design tradeoffs, validation limits, and future
  work sections.
- Make the final repository credible to embedded hardware, firmware, and
  systems-integration reviewers.

## Validation Strategy

- Use narrow checks first:
  - `git status --short`
  - tracked-file audits for generated artifacts
  - host tests once added
  - local firmware build if the STM32 toolchain is available
- Document hardware-only validation steps when hardware cannot be exercised
  directly.

## Assumptions

- Hardware validation is not available during this cleanup pass.
- CubeMX regeneration safety is more important than aggressive code movement.
- Unsupported hardware functions should be documented honestly rather than
  represented as working firmware features.
