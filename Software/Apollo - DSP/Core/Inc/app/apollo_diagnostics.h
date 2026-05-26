#ifndef APP_APOLLO_DIAGNOSTICS_H_
#define APP_APOLLO_DIAGNOSTICS_H_

#include <stdint.h>
#include "app/apollo_signal_chain.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t sample_count;
	uint32_t adc_error_count;
	uint32_t dac_error_count;
	uint32_t sram_error_count;
	uint32_t usb_busy_count;
	uint16_t min_adc;
	uint16_t max_adc;
	uint64_t sum_adc;
	uint64_t sum_squares_adc;
	apollo_signal_sample_t last_sample;
} apollo_diagnostics_t;

void apollo_diagnostics_reset(apollo_diagnostics_t *diagnostics);
void apollo_diagnostics_update_sample(apollo_diagnostics_t *diagnostics,
									  const apollo_signal_sample_t *sample);
void apollo_diagnostics_record_adc_error(apollo_diagnostics_t *diagnostics);
void apollo_diagnostics_record_dac_error(apollo_diagnostics_t *diagnostics);
void apollo_diagnostics_record_sram_error(apollo_diagnostics_t *diagnostics);
void apollo_diagnostics_record_usb_busy(apollo_diagnostics_t *diagnostics);
uint16_t apollo_diagnostics_mean_adc(const apollo_diagnostics_t *diagnostics);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_DIAGNOSTICS_H_ */
