#include "app/apollo_sd_format.h"

#include <stdio.h>

/* Highest index representable in the 8.3 name used below. */
#define APOLLO_SD_MAX_LOG_INDEX     99999UL

int apollo_sd_format_header(char *buffer, size_t length) {
	int written;

	if (buffer == NULL || length == 0U) {
		return -1;
	}

	written = snprintf(buffer, length,
					   "timestamp_ms,sequence,channel,raw_adc,filtered,dac_code,filter,flags\r\n");

	if (written < 0 || (size_t)written >= length) {
		return -1;
	}

	return written;
}

int apollo_sd_format_record(char *buffer, size_t length, const apollo_signal_sample_t *sample) {
	int written;

	if (buffer == NULL || sample == NULL || length == 0U) {
		return -1;
	}

	written = snprintf(buffer, length,
					   "%lu,%lu,%u,%u,%.4f,%u,%u,%lu\r\n",
					   (unsigned long)sample->timestamp_ms,
					   (unsigned long)sample->sequence,
					   (unsigned)sample->channel,
					   (unsigned)sample->raw_adc,
					   (double)sample->filtered,
					   (unsigned)sample->dac_code,
					   (unsigned)sample->filter_mode,
					   (unsigned long)sample->flags);

	if (written < 0 || (size_t)written >= length) {
		return -1;
	}

	return written;
}

int apollo_sd_format_filename(char *buffer, size_t length, uint32_t index) {
	int written;

	if (buffer == NULL || length == 0U || index > APOLLO_SD_MAX_LOG_INDEX) {
		return -1;
	}

	written = snprintf(buffer, length, "LOG%05lu.CSV", (unsigned long)index);

	if (written < 0 || (size_t)written >= length) {
		return -1;
	}

	return written;
}
