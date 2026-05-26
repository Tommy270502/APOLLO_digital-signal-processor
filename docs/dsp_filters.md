# DSP Filters

The DSP library in `dsp/dsp_filters` is designed for small embedded targets:
fixed-size buffers, no dynamic allocation, explicit parameter validation, and a
simple `init/reset/update` API.

| Mode | Command | Use case | Tradeoff |
| --- | --- | --- | --- |
| Bypass | `filter bypass` | Baseline raw signal path. | No noise reduction. |
| Low-pass IIR | `filter lowpass <hz>` | Smooth high-frequency noise. | Adds phase lag. |
| High-pass IIR | `filter highpass <hz>` | Remove DC or slow drift. | Step inputs decay toward zero. |
| EMA | `filter ema <alpha>` | Simple smoothing with direct coefficient control. | Alpha must be tuned manually. |
| Moving average | `filter average <n>` | Predictable finite-window smoothing. | More RAM and group delay as `n` grows. |
| Median | `filter median <n>` | Reject isolated spikes. | More CPU than averaging due to per-sample sorting. |

Maximum windows are 16 samples for moving average and 9 samples for median.
These limits keep RAM and CPU use bounded on the STM32F401.
