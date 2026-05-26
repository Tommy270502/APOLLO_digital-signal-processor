#include "dsp/dsp_filters.h"

#include <stddef.h>

#define DSP_TWO_PI_F    6.283185307f

static uint8_t dsp_filter_window_limit(dsp_filter_mode_t mode) {
	if (mode == DSP_FILTER_MODE_MEDIAN) {
		return DSP_FILTER_MEDIAN_MAX_WINDOW;
	}

	return DSP_FILTER_MOVING_AVERAGE_MAX_WINDOW;
}

static apollo_status_t dsp_filter_validate(const dsp_filter_config_t *config) {
	if (config == NULL || config->sample_period_s <= 0.0f) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	switch (config->mode) {
	case DSP_FILTER_MODE_BYPASS:
		return APOLLO_STATUS_OK;

	case DSP_FILTER_MODE_LOWPASS:
	case DSP_FILTER_MODE_HIGHPASS:
		if (config->cutoff_hz <= 0.0f) {
			return APOLLO_STATUS_INVALID_ARG;
		}
		return APOLLO_STATUS_OK;

	case DSP_FILTER_MODE_EMA:
		if (config->alpha <= 0.0f || config->alpha > 1.0f) {
			return APOLLO_STATUS_INVALID_ARG;
		}
		return APOLLO_STATUS_OK;

	case DSP_FILTER_MODE_MOVING_AVERAGE:
	case DSP_FILTER_MODE_MEDIAN:
		if (config->window_size == 0U ||
			config->window_size > dsp_filter_window_limit(config->mode)) {
			return APOLLO_STATUS_INVALID_ARG;
		}
		return APOLLO_STATUS_OK;

	default:
		return APOLLO_STATUS_INVALID_ARG;
	}
}

static void dsp_filter_clear_buffers(dsp_filter_t *filter, float value) {
	for (uint8_t i = 0U; i < DSP_FILTER_MOVING_AVERAGE_MAX_WINDOW; i++) {
		filter->moving_samples[i] = value;
	}

	for (uint8_t i = 0U; i < DSP_FILTER_MEDIAN_MAX_WINDOW; i++) {
		filter->median_samples[i] = value;
	}
}

static float dsp_filter_moving_average(dsp_filter_t *filter, float input) {
	float sum = 0.0f;
	uint8_t window = filter->config.window_size;

	filter->moving_samples[filter->sample_index] = input;
	filter->sample_index = (uint8_t)((filter->sample_index + 1U) % window);
	if (filter->sample_count < window) {
		filter->sample_count++;
	}

	for (uint8_t i = 0U; i < filter->sample_count; i++) {
		sum += filter->moving_samples[i];
	}

	return sum / (float)filter->sample_count;
}

static float dsp_filter_median(dsp_filter_t *filter, float input) {
	float sorted[DSP_FILTER_MEDIAN_MAX_WINDOW];
	uint8_t window = filter->config.window_size;

	filter->median_samples[filter->sample_index] = input;
	filter->sample_index = (uint8_t)((filter->sample_index + 1U) % window);
	if (filter->sample_count < window) {
		filter->sample_count++;
	}

	for (uint8_t i = 0U; i < filter->sample_count; i++) {
		sorted[i] = filter->median_samples[i];
	}

	for (uint8_t i = 1U; i < filter->sample_count; i++) {
		float key = sorted[i];
		uint8_t j = i;

		while (j > 0U && sorted[j - 1U] > key) {
			sorted[j] = sorted[j - 1U];
			j--;
		}
		sorted[j] = key;
	}

	return sorted[filter->sample_count / 2U];
}

void dsp_filter_default_config(dsp_filter_config_t *config) {
	if (config == NULL) {
		return;
	}

	config->mode = DSP_FILTER_MODE_LOWPASS;
	config->sample_period_s = 0.001f;
	config->cutoff_hz = 1000.0f;
	config->alpha = 0.1f;
	config->window_size = 5U;
}

apollo_status_t dsp_filter_init(dsp_filter_t *filter, const dsp_filter_config_t *config) {
	apollo_status_t status;
	float rc;

	if (filter == NULL || config == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	status = dsp_filter_validate(config);
	if (status != APOLLO_STATUS_OK) {
		return status;
	}

	filter->config = *config;
	filter->coeff_a = 1.0f;
	filter->coeff_b = 0.0f;
	filter->previous_input = 0.0f;
	filter->previous_output = 0.0f;
	filter->sample_index = 0U;
	filter->sample_count = 0U;
	dsp_filter_clear_buffers(filter, 0.0f);

	if (config->mode == DSP_FILTER_MODE_LOWPASS ||
		config->mode == DSP_FILTER_MODE_HIGHPASS) {
		rc = 1.0f / (DSP_TWO_PI_F * config->cutoff_hz);
		filter->coeff_a = config->sample_period_s / (config->sample_period_s + rc);
		filter->coeff_b = rc / (config->sample_period_s + rc);
	} else if (config->mode == DSP_FILTER_MODE_EMA) {
		filter->coeff_a = config->alpha;
		filter->coeff_b = 1.0f - config->alpha;
	}

	return APOLLO_STATUS_OK;
}

void dsp_filter_reset(dsp_filter_t *filter, float value) {
	if (filter == NULL) {
		return;
	}

	filter->previous_input = value;
	filter->previous_output = value;
	filter->sample_index = 0U;
	filter->sample_count = 0U;
	dsp_filter_clear_buffers(filter, value);
}

apollo_status_t dsp_filter_update(dsp_filter_t *filter, float input, float *output) {
	float y;

	if (filter == NULL || output == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	switch (filter->config.mode) {
	case DSP_FILTER_MODE_BYPASS:
		y = input;
		break;

	case DSP_FILTER_MODE_LOWPASS:
	case DSP_FILTER_MODE_EMA:
		y = (filter->coeff_a * input) + (filter->coeff_b * filter->previous_output);
		break;

	case DSP_FILTER_MODE_HIGHPASS:
		y = filter->coeff_b * (filter->previous_output + input - filter->previous_input);
		break;

	case DSP_FILTER_MODE_MOVING_AVERAGE:
		y = dsp_filter_moving_average(filter, input);
		break;

	case DSP_FILTER_MODE_MEDIAN:
		y = dsp_filter_median(filter, input);
		break;

	default:
		return APOLLO_STATUS_INVALID_ARG;
	}

	filter->previous_input = input;
	filter->previous_output = y;
	*output = y;

	return APOLLO_STATUS_OK;
}

const char *dsp_filter_name(dsp_filter_mode_t mode) {
	switch (mode) {
	case DSP_FILTER_MODE_BYPASS:
		return "bypass";
	case DSP_FILTER_MODE_LOWPASS:
		return "lowpass";
	case DSP_FILTER_MODE_HIGHPASS:
		return "highpass";
	case DSP_FILTER_MODE_EMA:
		return "ema";
	case DSP_FILTER_MODE_MOVING_AVERAGE:
		return "average";
	case DSP_FILTER_MODE_MEDIAN:
		return "median";
	default:
		return "unknown";
	}
}
