/*----------------------------------------------------------------------/
 / Low level disk I/O module function checker                            /
 /-----------------------------------------------------------------------/
 / WARNING: The data on the target drive will be lost!
 */

#include <stdio.h>
#include <string.h>
//#include "ff.h"         /* Declarations of sector size */
//#include "diskio.h"     /* Declarations of disk functions */
#include "hw_config.h"
#include "sd_card.h"

#include "main.h" //  HAL_GPIO_WritePin

#include "printf.h"

typedef unsigned int UINT; /* int must be 16-bit or 32-bit */
typedef unsigned char BYTE; /* char must be 8-bit */
typedef unsigned short WORD; /* 16-bit unsigned integer */
typedef unsigned long DWORD; /* 32-bit unsigned integer */
typedef uint64_t QWORD; /* 64-bit unsigned integer */
typedef WORD WCHAR; /* UTF-16 character type */

typedef QWORD LBA_t;

/* Status of Disk Functions */
typedef BYTE DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0, /* 0: Successful */
	RES_ERROR, /* 1: R/W Error */
	RES_WRPRT, /* 2: Write Protected */
	RES_NOTRDY, /* 3: Not Ready */
	RES_PARERR /* 4: Invalid Parameter */
} DRESULT;

#define FF_MAX_SS		512

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

#define TRACE_PRINTF printf

static int sdrc2dresult(int sd_rc) {
	switch (sd_rc) {
	case SD_BLOCK_DEVICE_ERROR_NONE:
		return RES_OK;
	case SD_BLOCK_DEVICE_ERROR_UNUSABLE:
	case SD_BLOCK_DEVICE_ERROR_NO_RESPONSE:
	case SD_BLOCK_DEVICE_ERROR_NO_INIT:
	case SD_BLOCK_DEVICE_ERROR_NO_DEVICE:
		return RES_NOTRDY;
	case SD_BLOCK_DEVICE_ERROR_PARAMETER:
	case SD_BLOCK_DEVICE_ERROR_UNSUPPORTED:
		return RES_PARERR;
	case SD_BLOCK_DEVICE_ERROR_WRITE_PROTECTED:
		return RES_WRPRT;
	case SD_BLOCK_DEVICE_ERROR_CRC:
	case SD_BLOCK_DEVICE_ERROR_WOULD_BLOCK:
	case SD_BLOCK_DEVICE_ERROR_ERASE:
	case SD_BLOCK_DEVICE_ERROR_WRITE:
	default:
		return RES_ERROR;
	}
}

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
	TRACE_PRINTF(">>> %s\r\n", __FUNCTION__);
	sd_card_t *p_sd = sd_get_by_num(pdrv);
	if (!p_sd)
		return RES_PARERR;
	return sd_init_card(p_sd); // See http://elm-chan.org/fsw/ff/doc/dstat.html
}
/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(BYTE pdrv, /* Physical drive nmuber to identify the drive */
BYTE *buff, /* Data buffer to store read data */
LBA_t sector, /* Start sector in LBA */
UINT count /* Number of sectors to read */
) {
	TRACE_PRINTF(">>> %s\r\n", __FUNCTION__);
	sd_card_t *p_sd = sd_get_by_num(pdrv);
	if (!p_sd)
		return RES_PARERR;
	int rc = sd_read_blocks(p_sd, buff, sector, count);
	return sdrc2dresult(rc);
}
/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
const BYTE *buff, /* Data to be written */
LBA_t sector, /* Start sector in LBA */
UINT count /* Number of sectors to write */
) {
	TRACE_PRINTF(">>> %s\r\n", __FUNCTION__);
	sd_card_t *p_sd = sd_get_by_num(pdrv);
	if (!p_sd)
		return RES_PARERR;
	int rc = sd_write_blocks(p_sd, buff, sector, count);
	return sdrc2dresult(rc);
}
DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
BYTE cmd, /* Control code */
void *buff /* Buffer to send/receive control data */
) {
	TRACE_PRINTF(">>> %s\r\n", __FUNCTION__);
	sd_card_t *p_sd = sd_get_by_num(pdrv);
	if (!p_sd)
		return RES_PARERR;
	switch (cmd) {
	case GET_SECTOR_COUNT: {  // Retrieves number of available sectors, the
							  // largest allowable LBA + 1, on the drive
							  // into the LBA_t variable pointed by buff.
							  // This command is used by f_mkfs and f_fdisk
							  // function to determine the size of
							  // volume/partition to be created. It is
							  // required when FF_USE_MKFS == 1.
		static LBA_t n;
		n = sd_sectors(p_sd);
		*(LBA_t*) buff = n;
		if (!n)
			return RES_ERROR;
		return RES_OK;
	}
	case GET_BLOCK_SIZE: {  // Retrieves erase block size of the flash
							// memory media in unit of sector into the DWORD
							// variable pointed by buff. The allowable value
							// is 1 to 32768 in power of 2. Return 1 if the
							// erase block size is unknown or non flash
							// memory media. This command is used by only
							// f_mkfs function and it attempts to align data
							// area on the erase block boundary. It is
							// required when FF_USE_MKFS == 1.
		static DWORD bs = 1;
		*(DWORD*) buff = bs;
		return RES_OK;
	}
	case GET_SECTOR_SIZE: {	// Retrieves sector size,
							// minimum data unit for generic read/write,
							// 	into the WORD variable that pointed by buff.
		static WORD ss = FF_MAX_SS;
		*(WORD*) buff = ss;
		return RES_OK;
	}
	case CTRL_SYNC:
		return RES_OK;
	default:
		return RES_PARERR;
	}
}

static DWORD pn( /* Pseudo random number generator */
DWORD pns /* 0:Initialize, !0:Read */
) {
	static DWORD lfsr;
	UINT n;

	if (pns) {
		lfsr = pns;
		for (n = 0; n < 32; n++)
			pn(0);
	}
	if (lfsr & 1) {
		lfsr >>= 1;
		lfsr ^= 0x80200003;
	} else {
		lfsr >>= 1;
	}
	return lfsr;
}

int test_diskio(BYTE pdrv, /* Physical drive number to be checked (all data on the drive will be lost) */
UINT ncyc, /* Number of test cycles */
DWORD *buff, /* Pointer to the working buffer */
UINT sz_buff /* Size of the working buffer in unit of byte */
) {
	UINT n, cc, ns;
	DWORD sz_drv, lba, lba2, sz_eblk, pns = 1;
	WORD sz_sect;
	BYTE *pbuff = (BYTE*) buff;
	DSTATUS ds;
	DRESULT dr;

	printf("test_diskio(%u, %u, %p, 0x%08X)\r\n", pdrv, ncyc, buff, sz_buff);

	if (sz_buff < FF_MAX_SS + 8) {
		printf("Insufficient work area to run the program.\r\n");
		return 1;
	}

	for (cc = 1; cc <= ncyc; cc++) {
		printf("**** Test cycle %u of %u start ****\r\n", cc, ncyc);

		printf(" disk_initalize(%u)", pdrv);
		ds = disk_initialize(pdrv);
		if (ds & STA_NOINIT) {
			printf(" - failed.\r\n");
			return 2;
		} else {
			printf(" - ok.\r\n");
		}

		printf("**** Get drive size ****\r\n");
		printf(" disk_ioctl(%u, GET_SECTOR_COUNT, 0x%08X)", pdrv,
				(UINT) &sz_drv);
		sz_drv = 0;
		dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 3;
		}
		if (sz_drv < 128) {
			printf("Failed: Insufficient drive size to test.\r\n");
			return 4;
		}
		printf(" Number of sectors on the drive %u is %lu.\r\n", pdrv, sz_drv);

#if FF_MAX_SS != FF_MIN_SS
		printf("**** Get sector size ****\r\n");
		printf(" disk_ioctl(%u, GET_SECTOR_SIZE, 0x%X)", pdrv, (UINT) &sz_sect);
		sz_sect = 0;
		dr = disk_ioctl(pdrv, GET_SECTOR_SIZE, &sz_sect);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 5;
		}
		printf(" Size of sector is %u bytes.\r\n", sz_sect);
#else
        sz_sect = FF_MAX_SS;
#endif

		printf("**** Get block size ****\r\n");
		printf(" disk_ioctl(%u, GET_BLOCK_SIZE, 0x%X)", pdrv, (UINT) &sz_eblk);
		sz_eblk = 0;
		dr = disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_eblk);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
		}
		if (dr == RES_OK || sz_eblk >= 2) {
			printf(" Size of the erase block is %lu sectors.\r\n", sz_eblk);
		} else {
			printf(" Size of the erase block is unknown.\r\n");
		}

		/* Single sector write test */
		printf("**** Single sector write test ****\r\n");
		lba = 0;
		for (n = 0, pn(pns); n < sz_sect; n++)
			pbuff[n] = (BYTE) pn(0);
		printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT) pbuff, lba);
		dr = disk_write(pdrv, pbuff, lba, 1);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 6;
		}
		printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
		dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 7;
		}
		memset(pbuff, 0, sz_sect);
		printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT) pbuff, lba);

		// Trigger:
		HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_SET);

		dr = disk_read(pdrv, pbuff, lba, 1);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 8;
		}
		for (n = 0, pn(pns); n < sz_sect && pbuff[n] == (BYTE) pn(0); n++)
			;
		if (n == sz_sect) {
			printf(" Read data matched.\r\n");
		} else {
			printf(" Read data differs from the data written.\r\n");
			return 10;
		}
		pns++;

// *** Begin CK3 mods
		printf("**** x sector write test ****\r\n");
		lba = 5;
		ns = 2;
		for (n = 0; n < sz_sect * ns; n++)
			if (n % 2)
				pbuff[n] = 0xA5;
			else
				pbuff[n] = 0x5A;
		printf(" disk_write(%u, 0x%X, %lu, %u)", pdrv, (UINT) pbuff, lba, ns);
		dr = disk_write(pdrv, pbuff, lba, ns);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 11;
		}
		printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
		dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 12;
		}
		memset(pbuff, 0, sz_sect * ns);
		printf(" disk_read(%u, 0x%X, %lu, %u)", pdrv, (UINT) pbuff, lba, ns);

		dr = disk_read(pdrv, pbuff, lba, ns);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 13;
		}
		for (n = 0; n < sz_sect * ns; n++) {
			if  ((n % 2 && pbuff[n] != 0xA5) && pbuff[n] != 0x5A) {
				printf("n: %u, pbuff[n]: 0x%0x\r\n", n, pbuff[n]);
				break;
			}
		}
		if (n == (UINT) (sz_sect * ns)) {
			printf(" Read data matched.\r\n");
		} else {
			printf(" Read data differs from the data written.\r\n");
			return 14;
		}
		pns++;
// *** End CK3 mods

		printf("**** Multiple sector write test ****\r\n");
		lba = 5;
		ns = sz_buff / sz_sect;
		if (ns > 4)
			ns = 4;
		if (ns > 1) {
			for (n = 0, pn(pns); n < (UINT) (sz_sect * ns); n++)
				pbuff[n] = (BYTE) pn(0);
			printf(" disk_write(%u, 0x%X, %lu, %u)", pdrv, (UINT) pbuff, lba,
					ns);
			dr = disk_write(pdrv, pbuff, lba, ns);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 11;
			}
			printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
			dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 12;
			}
			memset(pbuff, 0, sz_sect * ns);
			printf(" disk_read(%u, 0x%X, %lu, %u)", pdrv, (UINT) pbuff, lba,
					ns);
			dr = disk_read(pdrv, pbuff, lba, ns);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 13;
			}
			for (n = 0, pn(pns);
					n < (UINT) (sz_sect * ns) && pbuff[n] == (BYTE) pn(0); n++)
				;
			if (n == (UINT) (sz_sect * ns)) {
				printf(" Read data matched.\r\n");
			} else {
				printf(" Read data differs from the data written.\r\n");
				return 14;
			}
		} else {
			printf(" Test skipped.\r\n");
		}
		pns++;

		printf("**** Single sector write test (unaligned buffer address) ****\r\n");
		lba = 5;
		for (n = 0, pn(pns); n < sz_sect; n++)
			pbuff[n + 3] = (BYTE) pn(0);
		printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT) (pbuff + 3), lba);
		dr = disk_write(pdrv, pbuff + 3, lba, 1);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 15;
		}
		printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
		dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 16;
		}
		memset(pbuff + 5, 0, sz_sect);
		printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT) (pbuff + 5), lba);
		dr = disk_read(pdrv, pbuff + 5, lba, 1);
		if (dr == RES_OK) {
			printf(" - ok.\r\n");
		} else {
			printf(" - failed.\r\n");
			return 17;
		}
		for (n = 0, pn(pns); n < sz_sect && pbuff[n + 5] == (BYTE) pn(0); n++)
			;
		if (n == sz_sect) {
			printf(" Read data matched.\r\n");
		} else {
			printf(" Read data differs from the data written.\r\n");
			return 18;
		}
		pns++;

		printf("**** 4GB barrier test ****\r\n");
		if (sz_drv >= 128 + 0x80000000 / (sz_sect / 2)) {
			lba = 6;
			lba2 = lba + 0x80000000 / (sz_sect / 2);
			for (n = 0, pn(pns); n < (UINT) (sz_sect * 2); n++)
				pbuff[n] = (BYTE) pn(0);
			printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT) pbuff, lba);
			dr = disk_write(pdrv, pbuff, lba, 1);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 19;
			}
			printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv,
					(UINT) (pbuff + sz_sect), lba2);
			dr = disk_write(pdrv, pbuff + sz_sect, lba2, 1);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 20;
			}
			printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
			dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 21;
			}
			memset(pbuff, 0, sz_sect * 2);
			printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT) pbuff, lba);
			dr = disk_read(pdrv, pbuff, lba, 1);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 22;
			}
			printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv,
					(UINT) (pbuff + sz_sect), lba2);
			dr = disk_read(pdrv, pbuff + sz_sect, lba2, 1);
			if (dr == RES_OK) {
				printf(" - ok.\r\n");
			} else {
				printf(" - failed.\r\n");
				return 23;
			}
			for (n = 0, pn(pns);
					pbuff[n] == (BYTE) pn(0) && n < (UINT) (sz_sect * 2); n++)
				;
			if (n == (UINT) (sz_sect * 2)) {
				printf(" Read data matched.\r\n");
			} else {
				printf(" Read data differs from the data written.\r\n");
				return 24;
			}
		} else {
			printf(" Test skipped.\r\n");
		}
		pns++;

		printf("**** Test cycle %u of %u completed ****\n\r\n", cc, ncyc);
	}

	return 0;
}

//int main (int argc, char* argv[])
int lliot(size_t pnum) {
	int rc;
	DWORD buff[FF_MAX_SS] = { 0 }; /* Working buffer (4 sector in size) */

	/* Check function/compatibility of the physical drive #0 */
	rc = test_diskio(pnum, 3, buff, sizeof buff);

	if (rc) {
		printf(
				"Sorry the function/compatibility test failed. (rc=%d)\nFatFs will not work with this disk driver.\r\n",
				rc);
	} else {
		printf("Congratulations! The disk driver works well.\r\n");
	}

	return rc;
}

