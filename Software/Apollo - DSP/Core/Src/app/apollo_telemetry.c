#include "app/apollo_telemetry.h"

#include <stdio.h>
#include "dsp/dsp_filters.h"

int apollo_telemetry_format_sample(char *buffer,
								   size_t length,
								   const apollo_signal_sample_t *sample) {
	if (buffer == NULL || length == 0U || sample == NULL) {
		return -1;
	}

	return snprintf(buffer,
					length,
					"T,%lu,%lu,%u,%u,%.2f,%u,%s,0x%08lX\r\n",
					(unsigned long)sample->timestamp_ms,
					(unsigned long)sample->sequence,
					(unsigned int)sample->channel,
					(unsigned int)sample->raw_adc,
					(double)sample->filtered,
					(unsigned int)sample->dac_code,
					dsp_filter_name(sample->filter_mode),
					(unsigned long)sample->flags);
}

int apollo_telemetry_format_status(char *buffer,
								   size_t length,
								   const apollo_signal_chain_t *chain,
								   const apollo_diagnostics_t *diagnostics,
								   uint8_t telemetry_enabled,
								   uint8_t storage_available,
								   uint32_t rx_overflow_count) {
	if (buffer == NULL || length == 0U || chain == NULL || diagnostics == NULL) {
		return -1;
	}

	return snprintf(buffer,
					length,
					"status samples=%lu ch=%u filter=%s demo=%s telemetry=%s sram=%s"
					" adc_cal=%.3f/%.1f dac_cal=%.3f/%.1f"
					" min=%u max=%u mean=%u"
					" adc_err=%lu dac_err=%lu sram_err=%lu usb_busy=%lu rx_overflow=%lu\r\n",
					(unsigned long)diagnostics->sample_count,
					(unsigned int)chain->active_channel,
					dsp_filter_name(chain->filter_config.mode),
					apollo_demo_mode_name(chain->demo_mode),
					telemetry_enabled ? "on" : "off",
					storage_available ? "ok" : "unavailable",
					(double)chain->adc_gain, (double)chain->adc_offset,
					(double)chain->dac_gain, (double)chain->dac_offset,
					(unsigned int)diagnostics->min_adc,
					(unsigned int)diagnostics->max_adc,
					(unsigned int)apollo_diagnostics_mean_adc(diagnostics),
					(unsigned long)diagnostics->adc_error_count,
					(unsigned long)diagnostics->dac_error_count,
					(unsigned long)diagnostics->sram_error_count,
					(unsigned long)diagnostics->usb_busy_count,
					(unsigned long)rx_overflow_count);
}
