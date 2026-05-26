#include "platform/apollo_adc.h"

#include <stddef.h>

static apollo_status_t apollo_adc_configure_channel(apollo_adc_t *adc, uint8_t channel) {
	ADC_ChannelConfTypeDef config = {0};

	if (channel > 1U) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	config.Channel = (channel == 0U) ? ADC_CHANNEL_0 : ADC_CHANNEL_1;
	config.Rank = 1U;
	config.SamplingTime = ADC_SAMPLETIME_3CYCLES;

	if (HAL_ADC_ConfigChannel(adc->adc, &config) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	adc->active_channel = channel;
	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_adc_init(apollo_adc_t *adc, ADC_HandleTypeDef *hal_adc, uint32_t timeout_ms) {
	if (adc == NULL || hal_adc == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	adc->adc = hal_adc;
	adc->timeout_ms = timeout_ms;
	adc->active_channel = 0U;

	return apollo_adc_configure_channel(adc, 0U);
}

apollo_status_t apollo_adc_set_channel(apollo_adc_t *adc, uint8_t channel) {
	if (adc == NULL || adc->adc == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (adc->active_channel == channel) {
		return APOLLO_STATUS_OK;
	}

	(void)HAL_ADC_Stop(adc->adc);
	return apollo_adc_configure_channel(adc, channel);
}

apollo_status_t apollo_adc_read(apollo_adc_t *adc, uint16_t *sample) {
	apollo_status_t status = APOLLO_STATUS_OK;

	if (adc == NULL || adc->adc == NULL || sample == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (HAL_ADC_Start(adc->adc) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	if (HAL_ADC_PollForConversion(adc->adc, adc->timeout_ms) != HAL_OK) {
		status = APOLLO_STATUS_TIMEOUT;
	} else {
		*sample = (uint16_t)HAL_ADC_GetValue(adc->adc);
	}

	(void)HAL_ADC_Stop(adc->adc);
	return status;
}
