#ifndef DSP_DSP_FILTERS_H_
#define DSP_DSP_FILTERS_H_

#include <stdint.h>
#include "apollo_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_FILTER_MOVING_AVERAGE_MAX_WINDOW    16U
#define DSP_FILTER_MEDIAN_MAX_WINDOW            9U

typedef enum {
	DSP_FILTER_MODE_BYPASS = 0,
	DSP_FILTER_MODE_LOWPASS,
	DSP_FILTER_MODE_HIGHPASS,
	DSP_FILTER_MODE_EMA,
	DSP_FILTER_MODE_MOVING_AVERAGE,
	DSP_FILTER_MODE_MEDIAN
} dsp_filter_mode_t;

typedef struct {
	dsp_filter_mode_t mode;
	float sample_period_s;
	float cutoff_hz;
	float alpha;
	uint8_t window_size;
} dsp_filter_config_t;

typedef struct {
	dsp_filter_config_t config;
	float coeff_a;
	float coeff_b;
	float previous_input;
	float previous_output;
	float moving_samples[DSP_FILTER_MOVING_AVERAGE_MAX_WINDOW];
	float median_samples[DSP_FILTER_MEDIAN_MAX_WINDOW];
	uint8_t sample_index;
	uint8_t sample_count;
} dsp_filter_t;

void dsp_filter_default_config(dsp_filter_config_t *config);
apollo_status_t dsp_filter_init(dsp_filter_t *filter, const dsp_filter_config_t *config);
void dsp_filter_reset(dsp_filter_t *filter, float value);
apollo_status_t dsp_filter_update(dsp_filter_t *filter, float input, float *output);
const char *dsp_filter_name(dsp_filter_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* DSP_DSP_FILTERS_H_ */
