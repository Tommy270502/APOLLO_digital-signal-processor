# Demo

Connect the board over USB and open the CDC serial port. Line endings should
include `\n` or `\r\n`.

Useful commands:

```text
help
status
demo sine
filter lowpass 100
filter median 5
input 1
telemetry off
telemetry on
```

Telemetry frames:

```text
T,timestamp_ms,sequence,channel,raw_adc,filtered,dac_code,filter,flags
```

Demo modes generate synthetic ADC samples in firmware:

| Mode | Behavior |
| --- | --- |
| `demo sine` | 32-point sine-like waveform. |
| `demo step` | Alternates between low and high levels. |
| `demo impulse` | Single full-scale pulse every 64 samples. |
| `demo off` | Uses the selected ADC input. |

These modes make the firmware demonstrable without external analog signal
equipment. DAC and SRAM behavior still require hardware validation.
