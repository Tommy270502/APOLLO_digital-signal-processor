# CLAUDE.md

## Goal
Stabilize the refactored APOLLO DSP firmware, validate it, then add higher-risk features only after the current build and board behavior are proven.

## Rules
- Preserve the clean STM32CubeIDE build.
- Make the smallest correct change for the current phase.
- Do not refactor unrelated code.
- Do not claim DMA or microSD support until implemented and validated.
- Record only observed results in `docs/validation.md` and `README.md`.
- Keep host tests HAL-free; CI Linux is the canonical host-test target.
- If CI fails, fix host test/build code first, not firmware behavior.

## Phase Order
1. Stabilize current refactor.
2. Stabilize host tests and CI.
3. Run board smoke validation.
4. Harden USB/app robustness.
5. Add runtime calibration commands.
6. Add ADC timer/DMA only after smoke validation.
7. Decide microSD support only with real implementation.
8. Polish README after validation.

## Phase 1: Stabilize
- Review working tree and split into reviewable commits:
  - repository hygiene and `.gitignore`
  - firmware architecture refactor
  - DSP, CLI, telemetry, diagnostics
  - docs, tests, CI
- Confirm all new firmware sources are tracked.
- Confirm generated/local artifacts remain untracked or removed.
- Run:
  - `git diff --check`
  - audit `git ls-files` for `Debug/`, `Release/`, `.metadata/`, ZIPs, KiCad lock files
  - STM32CubeIDE Debug build
  - STM32CubeIDE Release build if configured
- Update `docs/validation.md` with confirmed build results.

## Phase 2: Host Tests/CI
- Target: `make -C tests/host test` passes on Ubuntu GitHub Actions.
- Test only pure logic:
  - DSP filters
  - command parser
  - telemetry formatting
  - diagnostics
  - clipping and demo signal paths
- Document Windows/MSYS host tests as optional.

## Phase 3: Board Smoke
- Flash CubeIDE-built firmware.
- Validate USB CDC:
  - enumeration
  - `help`
  - `status`
  - malformed command error handling
- Validate DSP/demo:
  - demo sine, step, impulse
  - bypass
  - lowpass 100
  - highpass 100
  - average 5
  - median 5
  - well-formed telemetry frames
- Validate analog:
  - input 0 and input 1
  - known voltage readings on both channels
  - DAC follows filtered/demo signal
- Validate SRAM:
  - self-test status
  - no increasing error count during normal logging
- Record results in `docs/validation.md`.

## Phase 4: Robustness
- USB CDC:
  - count RX overflows
  - count dropped TX frames when CDC is busy
  - expose both counters in status
- Startup:
  - report ADC, DAC, SRAM init state
  - keep running degraded if SRAM or DAC fails
  - never block forever on peripheral self-tests
- Add UART fallback telemetry only if USB validation proves it is needed.
- Update telemetry/status docs after validation.

## Phase 5: Calibration
- Add volatile commands only:
  - `calibrate clear`
  - `calibrate adc <gain> <offset>`
  - `calibrate dac <gain> <offset>`
- Validate:
  - gain positive and bounded
  - offset bounded to practical ADC/DAC code range
- Expose calibration in status.
- Add host tests for calibration parsing and signal-chain math.
- Persistence is out of scope.

## Phase 6: ADC Timer/DMA
- Keep bounded polling as default until board smoke passes.
- Add DMA as compile-time selectable acquisition mode.
- Requirements:
  - ADC scan CH0/CH1 only when both-channel periodic sampling is selected
  - timer trigger for fixed sample rate
  - DMA completion callbacks publish samples to app layer
  - track overruns, missed samples, last DMA error
  - preserve polling fallback
- Acceptance:
  - CubeIDE build passes
  - telemetry sample rate is stable
  - overrun count stays zero in normal demo use
  - docs include measured sample-rate behavior

## Phase 7: microSD
- Default status: unsupported.
- If implemented, use SPI SD block device plus FatFs.
- Minimum support claim requires:
  - card init
  - mount/open/write/close
  - CSV or binary log readable on PC
  - card removal/error reporting
- If not implemented, document as future work.

## Phase 8: README
- Update only with validated facts:
  - CubeIDE build status
  - host test/CI status
  - board smoke status
  - known hardware limits
  - measured telemetry frame
  - DAC observation
  - ADC channel notes

## Public Interfaces
- Keep `apollo_status_t` as shared status surface.
- Keep `dsp_filter_config_t` and `dsp_filter_t` as DSP API.
- Extend command handling only through `apollo_cli_command_t`.
- Do not parse commands directly inside `apollo_app`.
- Add calibration through `apollo_signal_chain_t` fields and expose via CLI/status.
- If DMA is added, create a platform acquisition interface; do not leak DMA state into `apollo_app`.

## Required Checks
- Host: `make -C tests/host test`
- CI: Ubuntu host-test job
- Firmware: STM32CubeIDE Debug build
- Firmware: STM32CubeIDE Release build if used
- Hardware: USB CDC, ADC CH0/CH1, DAC low/mid/high, SRAM, USB busy/error counters
- Docs: `README.md` and `docs/validation.md` reflect only observed results

## Final Acceptance
A reviewer can clone, build, understand, and assess the project within a few minutes.
