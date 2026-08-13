#include "app/apollo_sd_log.h"

#include <stddef.h>
#include <string.h>
#include "platform/sd_diskio.h"

/* Search bound when looking for an unused log file name. */
#define APOLLO_SD_MAX_FILE_SEARCH   1000U

/* Any FatFs failure is treated as the card having gone away: the logger stops
   recording and the card is marked absent so the sample loop stops retrying. */
static apollo_status_t sd_log_fail(apollo_sd_log_t *log) {
	log->state = APOLLO_SD_LOG_ERROR;
	log->write_errors++;

	if (log->card != NULL) {
		sd_card_deinit(log->card);
	}
	sd_diskio_attach(NULL);

	return APOLLO_STATUS_HAL_ERROR;
}

void apollo_sd_log_init(apollo_sd_log_t *log, sd_card_t *card) {
	if (log == NULL) {
		return;
	}

	memset(log, 0, sizeof(*log));
	log->card = card;
	log->state = APOLLO_SD_LOG_IDLE;
	log->filename[0] = '\0';
}

apollo_status_t apollo_sd_log_start(apollo_sd_log_t *log) {
	char header[APOLLO_SD_RECORD_LENGTH];
	int header_length;
	UINT written = 0U;

	if (log == NULL || log->card == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (log->state == APOLLO_SD_LOG_RECORDING) {
		return APOLLO_STATUS_OK;
	}

	if (sd_card_is_present(log->card) == 0U) {
		return APOLLO_STATUS_UNSUPPORTED;
	}

	sd_diskio_attach(log->card);

	if (log->state != APOLLO_SD_LOG_MOUNTED) {
		if (f_mount(&log->fs, "", 1U) != FR_OK) {
			return sd_log_fail(log);
		}
		log->state = APOLLO_SD_LOG_MOUNTED;
	}

	/* Take the first index whose file does not already exist, so repeated runs
	   do not overwrite earlier captures. */
	for (uint32_t index = 0U; index < APOLLO_SD_MAX_FILE_SEARCH; index++) {
		FILINFO info;

		if (apollo_sd_format_filename(log->filename, sizeof(log->filename), index) < 0) {
			return APOLLO_STATUS_INVALID_ARG;
		}

		if (f_stat(log->filename, &info) != FR_OK) {
			log->file_index = index;
			break;
		}

		if (index == (APOLLO_SD_MAX_FILE_SEARCH - 1U)) {
			return APOLLO_STATUS_UNSUPPORTED;
		}
	}

	if (f_open(&log->file, log->filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
		return sd_log_fail(log);
	}

	header_length = apollo_sd_format_header(header, sizeof(header));
	if (header_length < 0) {
		(void)f_close(&log->file);
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (f_write(&log->file, header, (UINT)header_length, &written) != FR_OK ||
		written != (UINT)header_length) {
		(void)f_close(&log->file);
		return sd_log_fail(log);
	}

	log->records_written = 0U;
	log->records_since_sync = 0U;
	log->state = APOLLO_SD_LOG_RECORDING;

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_sd_log_stop(apollo_sd_log_t *log) {
	if (log == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (log->state != APOLLO_SD_LOG_RECORDING) {
		return APOLLO_STATUS_OK;
	}

	if (f_close(&log->file) != FR_OK) {
		return sd_log_fail(log);
	}

	log->state = APOLLO_SD_LOG_MOUNTED;
	log->records_since_sync = 0U;

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_sd_log_write(apollo_sd_log_t *log, const apollo_signal_sample_t *sample) {
	char record[APOLLO_SD_RECORD_LENGTH];
	int record_length;
	UINT written = 0U;

	if (log == NULL || sample == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (log->state != APOLLO_SD_LOG_RECORDING) {
		return APOLLO_STATUS_UNSUPPORTED;
	}

	record_length = apollo_sd_format_record(record, sizeof(record), sample);
	if (record_length < 0) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (f_write(&log->file, record, (UINT)record_length, &written) != FR_OK ||
		written != (UINT)record_length) {
		return sd_log_fail(log);
	}

	log->records_written++;
	log->records_since_sync++;

	if (log->records_since_sync >= APOLLO_SD_SYNC_INTERVAL_RECORDS) {
		return apollo_sd_log_sync(log);
	}

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_sd_log_sync(apollo_sd_log_t *log) {
	if (log == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	if (log->state != APOLLO_SD_LOG_RECORDING) {
		return APOLLO_STATUS_UNSUPPORTED;
	}

	if (f_sync(&log->file) != FR_OK) {
		return sd_log_fail(log);
	}

	log->records_since_sync = 0U;

	return APOLLO_STATUS_OK;
}

apollo_sd_log_state_t apollo_sd_log_get_state(const apollo_sd_log_t *log) {
	return (log != NULL) ? log->state : APOLLO_SD_LOG_IDLE;
}

const char *apollo_sd_log_state_name(apollo_sd_log_state_t state) {
	switch (state) {
	case APOLLO_SD_LOG_MOUNTED:
		return "mounted";
	case APOLLO_SD_LOG_RECORDING:
		return "recording";
	case APOLLO_SD_LOG_ERROR:
		return "error";
	case APOLLO_SD_LOG_IDLE:
	default:
		return "idle";
	}
}

uint8_t apollo_sd_log_is_recording(const apollo_sd_log_t *log) {
	return (log != NULL && log->state == APOLLO_SD_LOG_RECORDING) ? 1U : 0U;
}

const char *apollo_sd_log_filename(const apollo_sd_log_t *log) {
	if (log == NULL || log->filename[0] == '\0') {
		return "-";
	}

	return log->filename;
}
