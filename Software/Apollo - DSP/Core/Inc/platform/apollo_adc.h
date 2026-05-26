#ifndef PLATFORM_APOLLO_ADC_H_
#define PLATFORM_APOLLO_ADC_H_

#include <stdint.h>
#include "apollo_status.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ADC_HandleTypeDef *adc;
	uint32_t timeout_ms;
	uint8_t active_channel;
} apollo_adc_t;

apollo_status_t apollo_adc_init(apollo_adc_t *adc, ADC_HandleTypeDef *hal_adc, uint32_t timeout_ms);
apollo_status_t apollo_adc_set_channel(apollo_adc_t *adc, uint8_t channel);
apollo_status_t apollo_adc_read(apollo_adc_t *adc, uint16_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_APOLLO_ADC_H_ */
