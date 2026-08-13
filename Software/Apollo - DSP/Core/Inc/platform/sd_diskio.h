#ifndef PLATFORM_SD_DISKIO_H_
#define PLATFORM_SD_DISKIO_H_

/*
 * Binds the single FatFs volume to an sd_card_t instance.
 *
 * ST's ff_gen_drv indirection is deliberately not used: this board has one
 * fixed drive, so diskio talks to the driver directly.
 */

#include "drivers/sd_card.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Must be called before f_mount. Passing NULL detaches the volume, which is
   how card removal is reported to FatFs. */
void sd_diskio_attach(sd_card_t *card);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SD_DISKIO_H_ */
