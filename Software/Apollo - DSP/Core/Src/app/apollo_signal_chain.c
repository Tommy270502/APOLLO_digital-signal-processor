#include "app/apollo_signal_chain.h"

#include <stddef.h>

static const uint16_t sine_table[32] = {
	2048U, 2447U, 2831U, 3185U, 3495U, 3750U, 3939U, 4056U,
	4095U, 4056U, 3939U, 3750U, 3495U, 3185U, 2831U, 2447U,
	2048U, 1648U, 1264U, 910U, 600U, 345U, 156U, 39U,
	0U, 39U, 156U, 345U, 600U, 910U, 1264U, 1648U
};

static uint16_t clamp_to_dac(float value, uint32_t *flags) {
	if (value <= 0.0f) {
		if (flags != NULL) {
			*flags |= APOLLO_SAMPLE_FLAG_DAC_CLIPPED_LOW;
		}
		return 0U;
	}

	if (value >= (float)APOLLO_DAC_MAX_CODE) {
		if (flags != NULL) {
			*flags |= APOLLO_SAMPLE_FLAG_DAC_CLIPPED_HIGH;
		}
		return APOLLO_DAC_MAX_CODE;
	}

	return (uint16_t)(value + 0.5f);
}

void apollo_signal_chain_init(apollo_signal_chain_t *chain) {
	dsp_filter_config_t config;

	if (chain == NULL) {
		return;
	}

	dsp_filter_default_config(&config);
	config.sample_period_s = APOLLO_DEFAULT_SAMPLE_PERIOD_S;
	config.cutoff_hz = APOLLO_DEFAULT_FILTER_CUTOFF_HZ;

	chain->filter_config = config;
	(void)dsp_filter_init(&chain->filter, &config);
	chain->demo_mode = APOLLO_DEMO_OFF;
	chain->active_channel = 0U;
	chain->adc_gain = 1.0f;
	chain->adc_offset = 0.0f;
	chain->dac_gain = 1.0f;
	chain->dac_offset = 0.0f;
	chain->demo_phase = 0U;
}

apollo_status_t apollo_signal_chain_set_filter(apollo_signal_chain_t *chain,
											   const dsp_filter_config_t *config) {
	apollo_status_t status;

	if (chain == NULL || config == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	status = dsp_filter_init(&chain->filter, config);
	if (status == APOLLO_STATUS_OK) {
		chain->filter_config = *config;
	}

	return status;
}

apollo_status_t apollo_signal_chain_set_channel(apollo_signal_chain_t *chain, uint8_t channel) {
	if (chain == NULL || channel > 1U) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	chain->active_channel = channel;
	return APOLLO_STATUS_OK;
}

void apollo_signal_chain_set_demo(apollo_signal_chain_t *chain, apollo_demo_mode_t mode) {
	if (chain == NULL) {
		return;
	}

	chain->demo_mode = mode;
	chain->demo_phase = 0U;
}

#define CALIBRATION_GAIN_MIN    0.01f
#define CALIBRATION_GAIN_MAX    100.0f
#define CALIBRATION_OFFSET_MAX  4095.0f

void apollo_signal_chain_clear_calibration(apollo_signal_chain_t *chain) {
	if (chain == NULL) {
		return;
	}

	chain->adc_gain = 1.0f;
	chain->adc_offset = 0.0f;
	chain->dac_gain = 1.0f;
	chain->dac_offset = 0.0f;
}

apollo_status_t apollo_signal_chain_set_adc_calibration(apollo_signal_chain_t *chain,
														float gain, float offset) {
	if (chain == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (gain < CALIBRATION_GAIN_MIN || gain > CALIBRATION_GAIN_MAX) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (offset < -CALIBRATION_OFFSET_MAX || offset > CALIBRATION_OFFSET_MAX) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	chain->adc_gain = gain;
	chain->adc_offset = offset;
	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_signal_chain_set_dac_calibration(apollo_signal_chain_t *chain,
														float gain, float offset) {
	if (chain == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (gain < CALIBRATION_GAIN_MIN || gain > CALIBRATION_GAIN_MAX) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (offset < -CALIBRATION_OFFSET_MAX || offset > CALIBRATION_OFFSET_MAX) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	chain->dac_gain = gain;
	chain->dac_offset = offset;
	return APOLLO_STATUS_OK;
}

uint16_t apollo_signal_chain_demo_sample(apollo_signal_chain_t *chain) {
	uint16_t sample = 0U;
	uint32_t phase;

	if (chain == NULL) {
		return 0U;
	}

	phase = chain->demo_phase++;

	switch (chain->demo_mode) {
	case APOLLO_DEMO_SINE:
		sample = sine_table[phase % 32U];
		break;

	case APOLLO_DEMO_STEP:
		sample = ((phase / 64U) & 1U) ? 3500U : 600U;
		break;

	case APOLLO_DEMO_IMPULSE:
		sample = ((phase % 64U) == 0U) ? 4095U : 0U;
		break;

	case APOLLO_DEMO_OFF:
	default:
		sample = 0U;
		break;
	}

	return sample;
}

apollo_status_t apollo_signal_chain_process(apollo_signal_chain_t *chain,
											uint32_t timestamp_ms,
											uint32_t sequence,
											uint16_t raw_adc,
											apollo_signal_sample_t *sample) {
	float calibrated_input;
	float filtered;
	apollo_status_t status;
	uint32_t flags = 0U;

	if (chain == NULL || sample == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	calibrated_input = ((float)raw_adc * chain->adc_gain) + chain->adc_offset;
	status = dsp_filter_update(&chain->filter, calibrated_input, &filtered);
	if (status != APOLLO_STATUS_OK) {
		filtered = calibrated_input;
		flags |= APOLLO_SAMPLE_FLAG_FILTER_ERROR;
	}

	if (chain->demo_mode != APOLLO_DEMO_OFF) {
		flags |= APOLLO_SAMPLE_FLAG_DEMO_SOURCE;
	}

	sample->timestamp_ms = timestamp_ms;
	sample->sequence = sequence;
	sample->channel = chain->active_channel;
	sample->raw_adc = raw_adc;
	sample->filtered = filtered;
	sample->dac_code = clamp_to_dac((filtered * chain->dac_gain) + chain->dac_offset, &flags);
	sample->filter_mode = chain->filter_config.mode;
	sample->flags = flags;

	return status;
}

const char *apollo_demo_mode_name(apollo_demo_mode_t mode) {
	switch (mode) {
	case APOLLO_DEMO_OFF:
		return "off";
	case APOLLO_DEMO_SINE:
		return "sine";
	case APOLLO_DEMO_STEP:
		return "step";
	case APOLLO_DEMO_IMPULSE:
		return "impulse";
	default:
		return "unknown";
	}
}
