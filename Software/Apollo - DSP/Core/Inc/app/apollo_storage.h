#ifndef APP_APOLLO_STORAGE_H_
#define APP_APOLLO_STORAGE_H_

#include <stdint.h>
#include "apollo_status.h"
#include "app/apollo_signal_chain.h"
#include "drivers/sram_23k256.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	sram_23k256_t *sram;
	uint16_t next_address;
	uint8_t available;
} apollo_storage_t;

void apollo_storage_init(apollo_storage_t *storage, sram_23k256_t *sram, uint8_t available);
apollo_status_t apollo_storage_log_sample(apollo_storage_t *storage,
										  const apollo_signal_sample_t *sample);
uint8_t apollo_storage_is_available(const apollo_storage_t *storage);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_STORAGE_H_ */
