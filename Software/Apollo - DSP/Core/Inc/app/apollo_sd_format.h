#ifndef APP_APOLLO_SD_FORMAT_H_
#define APP_APOLLO_SD_FORMAT_H_

/*
 * CSV record formatting for the microSD log.
 *
 * Kept free of both HAL and FatFs so the host test suite can cover the record
 * layout without a card or a filesystem.
 */

#include <stddef.h>
#include <stdint.h>
#include "app/apollo_signal_chain.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Longest record this module can emit, including the terminating newline and
   NUL. Sized for the worst case of every field at full width. */
#define APOLLO_SD_RECORD_LENGTH     128U

/* Column header written once at the top of each log file. */
int apollo_sd_format_header(char *buffer, size_t length);

/* One sample as a CSV row terminated with \r\n so the file opens cleanly on
   Windows as well as Unix. Returns the number of characters written, or a
   negative value if the buffer is too small. */
int apollo_sd_format_record(char *buffer, size_t length, const apollo_signal_sample_t *sample);

/* Builds an 8.3 log file name for the given index, e.g. index 12 gives
   "LOG00012.CSV". Returns characters written, or negative on bad input. */
int apollo_sd_format_filename(char *buffer, size_t length, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_SD_FORMAT_H_ */
