#include "platform/sd_diskio.h"

#include <stddef.h>
#include "diskio.h"
#include "ff.h"

/* Single fixed volume, so the drive number is only ever 0. */
#define SD_DISKIO_DRIVE     0U

static sd_card_t *sd_disk = NULL;

void sd_diskio_attach(sd_card_t *card) {
	sd_disk = card;
}

DSTATUS disk_status(BYTE pdrv) {
	if (pdrv != SD_DISKIO_DRIVE || sd_disk == NULL) {
		return STA_NOINIT;
	}

	return (sd_card_is_present(sd_disk) != 0U) ? (DSTATUS)0 : STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv) {
	/* The card is brought up by the application before f_mount, so this only
	   reports the current state rather than re-running the init sequence on
	   a bus that is shared with the SRAM. */
	return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
	if (pdrv != SD_DISKIO_DRIVE || sd_disk == NULL || buff == NULL) {
		return RES_PARERR;
	}
	if (sd_card_is_present(sd_disk) == 0U) {
		return RES_NOTRDY;
	}

	if (sd_card_read_blocks(sd_disk, (uint32_t)sector, (uint8_t *)buff, (uint32_t)count) != APOLLO_STATUS_OK) {
		return RES_ERROR;
	}

	return RES_OK;
}

#if !_FS_READONLY
DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
	if (pdrv != SD_DISKIO_DRIVE || sd_disk == NULL || buff == NULL) {
		return RES_PARERR;
	}
	if (sd_card_is_present(sd_disk) == 0U) {
		return RES_NOTRDY;
	}

	if (sd_card_write_blocks(sd_disk, (uint32_t)sector, (const uint8_t *)buff, (uint32_t)count) != APOLLO_STATUS_OK) {
		return RES_ERROR;
	}

	return RES_OK;
}
#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
	if (pdrv != SD_DISKIO_DRIVE || sd_disk == NULL) {
		return RES_PARERR;
	}
	if (sd_card_is_present(sd_disk) == 0U) {
		return RES_NOTRDY;
	}

	switch (cmd) {
	case CTRL_SYNC:
		return (sd_card_sync(sd_disk) == APOLLO_STATUS_OK) ? RES_OK : RES_ERROR;

	case GET_SECTOR_COUNT:
		if (buff == NULL) {
			return RES_PARERR;
		}
		*(DWORD *)buff = (DWORD)sd_card_sector_count(sd_disk);
		return RES_OK;

	case GET_SECTOR_SIZE:
		if (buff == NULL) {
			return RES_PARERR;
		}
		*(WORD *)buff = (WORD)SD_BLOCK_SIZE;
		return RES_OK;

	case GET_BLOCK_SIZE:
		if (buff == NULL) {
			return RES_PARERR;
		}
		/* Erase block size in sectors; 1 is always safe. */
		*(DWORD *)buff = 1U;
		return RES_OK;

	default:
		return RES_PARERR;
	}
}
