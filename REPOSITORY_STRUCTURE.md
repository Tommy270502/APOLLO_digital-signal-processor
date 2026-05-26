# Repository Structure

APOLLO is organized so hardware design assets, firmware source, validation
material, and public documentation are easy to inspect independently.

| Path | Purpose |
| --- | --- |
| `Apollo - DSP.kicad_pro` | Main KiCad project file. |
| `Apollo - DSP.kicad_sch` | Top-level schematic. |
| `Analog.kicad_sch` | Analog input/output and external DAC circuitry. |
| `DSP.kicad_sch` | STM32F401, USB, memory, SD-card socket, SWD, and UART circuitry. |
| `Apollo - DSP.kicad_pcb` | Routed PCB layout. |
| `Gerber/` | Fabrication outputs and drill files. |
| `pictures/` | Board images used by the public documentation. |
| `datasheets/` | Local component datasheets and reference material. |
| `Software/Apollo - DSP/` | STM32CubeIDE firmware project. |
| `Software/Apollo - DSP/Core/` | Firmware startup, generated HAL hooks, and application source. |
| `Software/Apollo - DSP/Core/Inc/app` | Application interfaces, command parser, signal chain, telemetry, diagnostics, and storage APIs. |
| `Software/Apollo - DSP/Core/Inc/dsp` | Reusable DSP filter interfaces. |
| `Software/Apollo - DSP/Core/Inc/drivers` | Board-level DAC, SRAM, and SD-card status drivers. |
| `Software/Apollo - DSP/Core/Inc/platform` | HAL-facing acquisition and USB CDC helpers. |
| `Software/Apollo - DSP/Drivers/` | STM32 HAL and CMSIS driver sources generated/copied by CubeMX. |
| `Software/Apollo - DSP/Middlewares/` | STM32 USB Device middleware. |
| `Software/Apollo - DSP/USB_DEVICE/` | CubeMX USB CDC device glue code. |
| `tests/host/` | Host-buildable tests for pure firmware logic. |
| `docs/` | Portfolio plan and technical documentation. |

Generated build outputs, IDE workspaces, KiCad lock/cache files, and packaged
snapshots are intentionally ignored and should be regenerated locally as needed.
