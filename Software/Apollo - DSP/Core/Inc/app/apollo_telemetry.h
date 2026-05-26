#ifndef APP_APOLLO_TELEMETRY_H_
#define APP_APOLLO_TELEMETRY_H_

#include <stddef.h>
#include "app/apollo_diagnostics.h"
#include "app/apollo_signal_chain.h"

#ifdef __cplusplus
extern "C" {
#endif

int apollo_telemetry_format_sample(char *buffer,
								   size_t length,
								   const apollo_signal_sample_t *sample);
int apollo_telemetry_format_status(char *buffer,
								   size_t length,
								   const apollo_signal_chain_t *chain,
								   const apollo_diagnostics_t *diagnostics,
								   uint8_t telemetry_enabled,
								   uint8_t storage_available);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_TELEMETRY_H_ */
