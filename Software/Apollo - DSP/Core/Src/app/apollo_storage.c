#include "app/apollo_storage.h"

#include <stddef.h>

static void pack_u16(uint8_t *buffer, uint16_t value) {
	buffer[0] = (uint8_t)(value & 0xFFU);
	buffer[1] = (uint8_t)(value >> 8);
}

static void pack_u32(uint8_t *buffer, uint32_t value) {
	buffer[0] = (uint8_t)(value & 0xFFU);
	buffer[1] = (uint8_t)((value >> 8) & 0xFFU);
	buffer[2] = (uint8_t)((value >> 16) & 0xFFU);
	buffer[3] = (uint8_t)((value >> 24) & 0xFFU);
}

void apollo_storage_init(apollo_storage_t *storage, sram_23k256_t *sram, uint8_t available) {
	if (storage == NULL) {
		return;
	}

	storage->sram = sram;
	storage->next_address = 0U;
	storage->available = available;
}

apollo_status_t apollo_storage_log_sample(apollo_storage_t *storage,
										  const apollo_signal_sample_t *sample) {
	uint8_t record[APOLLO_SRAM_LOG_RECORD_BYTES];
	apollo_status_t status;

	if (storage == NULL || sample == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (storage->available == 0U || storage->sram == NULL) {
		return APOLLO_STATUS_UNSUPPORTED;
	}

	pack_u32(&record[0], sample->timestamp_ms);
	pack_u16(&record[4], sample->raw_adc);
	pack_u16(&record[6], sample->dac_code);

	status = sram_23k256_write(storage->sram,
							   storage->next_address,
							   record,
							   (uint16_t)sizeof(record));
	if (status != APOLLO_STATUS_OK) {
		storage->available = 0U;
		return status;
	}

	storage->next_address = (uint16_t)(storage->next_address + (uint16_t)sizeof(record));
	if (storage->next_address > (SRAM_23K256_SIZE_BYTES - APOLLO_SRAM_LOG_RECORD_BYTES)) {
		storage->next_address = 0U;
	}

	return APOLLO_STATUS_OK;
}

uint8_t apollo_storage_is_available(const apollo_storage_t *storage) {
	if (storage == NULL) {
		return 0U;
	}

	return storage->available;
}
