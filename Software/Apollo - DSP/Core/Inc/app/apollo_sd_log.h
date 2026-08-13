#ifndef APP_APOLLO_SD_LOG_H_
#define APP_APOLLO_SD_LOG_H_

/*
 * CSV logging to microSD.
 *
 * Owns the FatFs volume lifecycle: mount, pick an unused 8.3 file name, open,
 * append, periodically sync, close. Any card error drops the logger back to
 * the idle state and marks the card absent, so a card pulled mid-run degrades
 * to "not logging" rather than blocking the sample loop.
 */

#include <stdint.h>
#include "apollo_status.h"
#include "app/apollo_sd_format.h"
#include "app/apollo_signal_chain.h"
#include "drivers/sd_card.h"
#include "ff.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Records buffered between f_sync calls. Syncing costs a metadata write, so
   this trades a bounded amount of data loss on power cut against wear. */
#define APOLLO_SD_SYNC_INTERVAL_RECORDS     64U

typedef enum {
	APOLLO_SD_LOG_IDLE = 0,      /* no card, or card present but not logging */
	APOLLO_SD_LOG_MOUNTED,       /* volume mounted, no file open */
	APOLLO_SD_LOG_RECORDING,     /* file open and accepting samples */
	APOLLO_SD_LOG_ERROR          /* last operation failed; card marked absent */
} apollo_sd_log_state_t;

typedef struct {
	sd_card_t *card;
	FATFS fs;
	FIL file;
	apollo_sd_log_state_t state;
	uint32_t records_written;
	uint32_t records_since_sync;
	uint32_t write_errors;
	uint32_t file_index;
	char filename[13];           /* 8.3 plus NUL */
} apollo_sd_log_t;

void apollo_sd_log_init(apollo_sd_log_t *log, sd_card_t *card);

/* Mounts the volume and opens the next free log file. */
apollo_status_t apollo_sd_log_start(apollo_sd_log_t *log);

/* Flushes and closes the current file, leaving the volume mounted. */
apollo_status_t apollo_sd_log_stop(apollo_sd_log_t *log);

/* Appends one sample. Cheap no-op unless the logger is recording. */
apollo_status_t apollo_sd_log_write(apollo_sd_log_t *log, const apollo_signal_sample_t *sample);

/* Forces buffered data and directory metadata to the card. */
apollo_status_t apollo_sd_log_sync(apollo_sd_log_t *log);

apollo_sd_log_state_t apollo_sd_log_get_state(const apollo_sd_log_t *log);
const char *apollo_sd_log_state_name(apollo_sd_log_state_t state);
uint8_t apollo_sd_log_is_recording(const apollo_sd_log_t *log);
const char *apollo_sd_log_filename(const apollo_sd_log_t *log);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_SD_LOG_H_ */
