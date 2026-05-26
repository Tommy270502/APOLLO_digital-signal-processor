#ifndef APP_APOLLO_SIGNAL_CHAIN_H_
#define APP_APOLLO_SIGNAL_CHAIN_H_

#include <stdint.h>
#include "apollo_status.h"
#include "app/apollo_config.h"
#include "dsp/dsp_filters.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APOLLO_SAMPLE_FLAG_DAC_CLIPPED_LOW      (1UL << 0)
#define APOLLO_SAMPLE_FLAG_DAC_CLIPPED_HIGH     (1UL << 1)
#define APOLLO_SAMPLE_FLAG_DEMO_SOURCE          (1UL << 2)
#define APOLLO_SAMPLE_FLAG_FILTER_ERROR         (1UL << 3)
#define APOLLO_SAMPLE_FLAG_ADC_ERROR            (1UL << 4)
#define APOLLO_SAMPLE_FLAG_DAC_ERROR            (1UL << 5)
#define APOLLO_SAMPLE_FLAG_SRAM_ERROR           (1UL << 6)

typedef enum {
	APOLLO_DEMO_OFF = 0,
	APOLLO_DEMO_SINE,
	APOLLO_DEMO_STEP,
	APOLLO_DEMO_IMPULSE
} apollo_demo_mode_t;

typedef struct {
	uint32_t timestamp_ms;
	uint32_t sequence;
	uint8_t channel;
	uint16_t raw_adc;
	float filtered;
	uint16_t dac_code;
	dsp_filter_mode_t filter_mode;
	uint32_t flags;
} apollo_signal_sample_t;

typedef struct {
	dsp_filter_t filter;
	dsp_filter_config_t filter_config;
	apollo_demo_mode_t demo_mode;
	uint8_t active_channel;
	float adc_gain;
	float adc_offset;
	float dac_gain;
	float dac_offset;
	uint32_t demo_phase;
} apollo_signal_chain_t;

void apollo_signal_chain_init(apollo_signal_chain_t *chain);
apollo_status_t apollo_signal_chain_set_filter(apollo_signal_chain_t *chain,
											   const dsp_filter_config_t *config);
apollo_status_t apollo_signal_chain_set_channel(apollo_signal_chain_t *chain, uint8_t channel);
void apollo_signal_chain_set_demo(apollo_signal_chain_t *chain, apollo_demo_mode_t mode);
void apollo_signal_chain_clear_calibration(apollo_signal_chain_t *chain);
uint16_t apollo_signal_chain_demo_sample(apollo_signal_chain_t *chain);
apollo_status_t apollo_signal_chain_process(apollo_signal_chain_t *chain,
											uint32_t timestamp_ms,
											uint32_t sequence,
											uint16_t raw_adc,
											apollo_signal_sample_t *sample);
const char *apollo_demo_mode_name(apollo_demo_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_SIGNAL_CHAIN_H_ */
