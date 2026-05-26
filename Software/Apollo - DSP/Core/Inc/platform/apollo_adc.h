#ifndef PLATFORM_APOLLO_ADC_H_
#define PLATFORM_APOLLO_ADC_H_

#include <stdint.h>
#include "apollo_status.h"
#include "stm32f4xx_hal.h"

#ifndef APOLLO_ADC_USE_DMA
#define APOLLO_ADC_USE_DMA  0
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t overruns;
	uint32_t missed_samples;
	uint32_t last_error;
} apollo_adc_stats_t;

typedef struct {
	ADC_HandleTypeDef *adc;
	uint32_t timeout_ms;
	uint8_t active_channel;
#if APOLLO_ADC_USE_DMA
	volatile uint32_t dma_buffer;
	volatile uint8_t dma_ready;
	apollo_adc_stats_t stats;
#endif
} apollo_adc_t;

apollo_status_t apollo_adc_init(apollo_adc_t *adc, ADC_HandleTypeDef *hal_adc, uint32_t timeout_ms);
apollo_status_t apollo_adc_set_channel(apollo_adc_t *adc, uint8_t channel);
apollo_status_t apollo_adc_read(apollo_adc_t *adc, uint16_t *sample);
void apollo_adc_get_stats(const apollo_adc_t *adc, apollo_adc_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_APOLLO_ADC_H_ */
