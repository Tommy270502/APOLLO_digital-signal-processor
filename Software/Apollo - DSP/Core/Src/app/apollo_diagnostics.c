#include "app/apollo_diagnostics.h"

#include <stddef.h>

void apollo_diagnostics_reset(apollo_diagnostics_t *diagnostics) {
	if (diagnostics == NULL) {
		return;
	}

	diagnostics->sample_count = 0U;
	diagnostics->adc_error_count = 0U;
	diagnostics->dac_error_count = 0U;
	diagnostics->sram_error_count = 0U;
	diagnostics->usb_busy_count = 0U;
	diagnostics->min_adc = APOLLO_ADC_MAX_CODE;
	diagnostics->max_adc = 0U;
	diagnostics->sum_adc = 0U;
	diagnostics->sum_squares_adc = 0U;
	diagnostics->last_sample.timestamp_ms = 0U;
	diagnostics->last_sample.sequence = 0U;
	diagnostics->last_sample.channel = 0U;
	diagnostics->last_sample.raw_adc = 0U;
	diagnostics->last_sample.filtered = 0.0f;
	diagnostics->last_sample.dac_code = 0U;
	diagnostics->last_sample.filter_mode = DSP_FILTER_MODE_BYPASS;
	diagnostics->last_sample.flags = 0U;
}

void apollo_diagnostics_update_sample(apollo_diagnostics_t *diagnostics,
									  const apollo_signal_sample_t *sample) {
	uint32_t raw;

	if (diagnostics == NULL || sample == NULL) {
		return;
	}

	raw = sample->raw_adc;
	if (raw < diagnostics->min_adc) {
		diagnostics->min_adc = (uint16_t)raw;
	}
	if (raw > diagnostics->max_adc) {
		diagnostics->max_adc = (uint16_t)raw;
	}

	diagnostics->sample_count++;
	diagnostics->sum_adc += raw;
	diagnostics->sum_squares_adc += (uint64_t)raw * (uint64_t)raw;
	diagnostics->last_sample = *sample;
}

void apollo_diagnostics_record_adc_error(apollo_diagnostics_t *diagnostics) {
	if (diagnostics != NULL) {
		diagnostics->adc_error_count++;
	}
}

void apollo_diagnostics_record_dac_error(apollo_diagnostics_t *diagnostics) {
	if (diagnostics != NULL) {
		diagnostics->dac_error_count++;
	}
}

void apollo_diagnostics_record_sram_error(apollo_diagnostics_t *diagnostics) {
	if (diagnostics != NULL) {
		diagnostics->sram_error_count++;
	}
}

void apollo_diagnostics_record_usb_busy(apollo_diagnostics_t *diagnostics) {
	if (diagnostics != NULL) {
		diagnostics->usb_busy_count++;
	}
}

uint16_t apollo_diagnostics_mean_adc(const apollo_diagnostics_t *diagnostics) {
	if (diagnostics == NULL || diagnostics->sample_count == 0U) {
		return 0U;
	}

	return (uint16_t)(diagnostics->sum_adc / diagnostics->sample_count);
}
