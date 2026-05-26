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

#if APOLLO_ADC_USE_DMA

static apollo_adc_t *s_dma_instance;

apollo_status_t apollo_adc_init(apollo_adc_t *adc, ADC_HandleTypeDef *hal_adc, uint32_t timeout_ms) {
	if (adc == NULL || hal_adc == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	adc->adc = hal_adc;
	adc->timeout_ms = timeout_ms;
	adc->active_channel = 0U;
	adc->dma_buffer = 0U;
	adc->dma_ready = 0U;
	adc->stats.overruns = 0U;
	adc->stats.missed_samples = 0U;
	adc->stats.last_error = 0U;

	s_dma_instance = adc;

	apollo_status_t status = apollo_adc_configure_channel(adc, 0U);
	if (status != APOLLO_STATUS_OK) {
		return status;
	}

	if (HAL_ADC_Start_DMA(adc->adc, (uint32_t *)&adc->dma_buffer, 1U) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_adc_set_channel(apollo_adc_t *adc, uint8_t channel) {
	if (adc == NULL || adc->adc == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (adc->active_channel == channel) {
		return APOLLO_STATUS_OK;
	}

	(void)HAL_ADC_Stop_DMA(adc->adc);

	apollo_status_t status = apollo_adc_configure_channel(adc, channel);
	if (status != APOLLO_STATUS_OK) {
		return status;
	}

	adc->dma_ready = 0U;
	if (HAL_ADC_Start_DMA(adc->adc, (uint32_t *)&adc->dma_buffer, 1U) != HAL_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_adc_read(apollo_adc_t *adc, uint16_t *sample) {
	if (adc == NULL || adc->adc == NULL || sample == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	*sample = (uint16_t)(adc->dma_buffer & 0xFFFFU);

	if (adc->dma_ready != 0U) {
		adc->dma_ready = 0U;
		(void)HAL_ADC_Start_DMA(adc->adc, (uint32_t *)&adc->dma_buffer, 1U);
	} else {
		adc->stats.missed_samples++;
	}

	return APOLLO_STATUS_OK;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (s_dma_instance != NULL && s_dma_instance->adc == hadc) {
		if (s_dma_instance->dma_ready != 0U) {
			s_dma_instance->stats.overruns++;
		}
		s_dma_instance->dma_ready = 1U;
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
	if (s_dma_instance != NULL && s_dma_instance->adc == hadc) {
		s_dma_instance->stats.last_error = HAL_ADC_GetError(hadc);
	}
}

void apollo_adc_get_stats(const apollo_adc_t *adc, apollo_adc_stats_t *stats) {
	if (adc == NULL || stats == NULL) {
		return;
	}
	*stats = adc->stats;
}

#else /* polling mode */

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

void apollo_adc_get_stats(const apollo_adc_t *adc, apollo_adc_stats_t *stats) {
	if (stats == NULL) {
		return;
	}
	(void)adc;
	stats->overruns = 0U;
	stats->missed_samples = 0U;
	stats->last_error = 0U;
}

#endif /* APOLLO_ADC_USE_DMA */
