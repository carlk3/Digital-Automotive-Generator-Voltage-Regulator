/*
 * command.c
 *
 *  Created on: Jan 5, 2022
 *      Author: carlk
 */

//#include "stm32l4xx_hal_rtc.h"
#include <string.h>
#include <stdlib.h>
//
#include "hw_config.h"
#include "sd_card.h"
#include "fs.h"
#include "rtc.h"
#include "main.h"
//
#include "printf.h"

#define myASSERT configASSERT

static void run_setrtc() {
	const char *dateStr = strtok(NULL, " ");
	if (!dateStr) {
		printf("Missing argument\r\n");
		return;
	}
	int date = atoi(dateStr);

	const char *monthStr = strtok(NULL, " ");
	if (!monthStr) {
		printf("Missing argument\r\n");
		return;
	}
	int month = atoi(monthStr);

	const char *yearStr = strtok(NULL, " ");
	if (!yearStr) {
		printf("Missing argument\r\n");
		return;
	}
	int year = atoi(yearStr);

	const char *hourStr = strtok(NULL, " ");
	if (!hourStr) {
		printf("Missing argument\r\n");
		return;
	}
	int hour = atoi(hourStr);

	const char *minStr = strtok(NULL, " ");
	if (!minStr) {
		printf("Missing argument\r\n");
		return;
	};
	int min = atoi(minStr);

	const char *secStr = strtok(NULL, " ");
	if (!secStr) {
		printf("Missing argument\r\n");
		return;
	}
	int sec = atoi(secStr);

	struct tm timeinfo = {
	.tm_sec = sec, /* Seconds.	[0-60] (1 leap second) */
	.tm_min = min, /* Minutes.	[0-59] */
	.tm_hour = hour, /* Hours.	[0-23] */
	.tm_mday = date, /* Day.		[1-31] */
	.tm_mon = month - 1, /* Month.	[0-11] */
	.tm_year = year + 2000 - 1900, /* Year	- 1900.  */
	.tm_wday = 0, /* Day of week.	[0-6] */
	.tm_yday = 0, /* Days in year.[0-365]	*/
	.tm_isdst = -1 /* DST.		[-1/0/1]*/
	};
	mktime(&timeinfo);

    RTC_DateTypeDef dtd = {
    		.Year = year, /*!< Specifies the RTC Date Year.
                         This parameter must be a number between Min_Data = 0 and Max_Data = 99 */
			.Month = month,
			.Date = date,
			.WeekDay = timeinfo.tm_wday + 1
    };
	// HAL_StatusTypeDef HAL_RTC_SetDate (RTC_HandleTypeDef * hrtc, RTC_DateTypeDef * sDate, uint32_t Format)
	if (HAL_OK != HAL_RTC_SetDate(&hrtc, &dtd, RTC_FORMAT_BIN))
		Error_Handler();

    RTC_TimeTypeDef ttd = {
    		.Hours = hour, /* This parameter must be a number between Min_Data = 0 and Max_Data = 23 if the RTC_HourFormat_24 is selected */
			.Minutes = min,
			.Seconds = sec,
			.SubSeconds = 0,
			.SecondFraction = 0,
			.TimeFormat = RTC_HOURFORMAT12_AM,
			.DayLightSaving = 0,
			.StoreOperation = 0
    };
	//HAL_StatusTypeDef HAL_RTC_SetTime (RTC_HandleTypeDef * hrtc, RTC_TimeTypeDef * sTime, uint32_t Format)
	if (HAL_OK != HAL_RTC_SetTime(&hrtc, &ttd, RTC_FORMAT_BIN))
		Error_Handler();
}
static void run_lliot() {
	size_t pnum = 0;
	char *arg1 = strtok(NULL, " ");
	if (arg1) {
		pnum = strtoul(arg1, NULL, 0);
	}
	extern int lliot(size_t pnum);
	lliot(pnum);
}
static void run_simple() {
	fs_test();
}
static void run_date() {
	char buf[128] = { 0 };
	time_t epoch_secs = time(NULL);
	struct tm *ptm = localtime(&epoch_secs);
	size_t n = strftime(buf, sizeof(buf), "%c", ptm);
	myASSERT(n);
	printf("%s\r\n", buf);
	strftime(buf, sizeof(buf), "%j", ptm); // The day of the year as a decimal number (range
										   // 001 to 366).
	printf("Day of year: %s\r\n", buf);
}

static void run_format() {
	if (!fs_init()) return;
	/* Format the drive with default parameters */
	int err = lfs_format(&lfs, &cfg);
	if (LFS_ERR_OK != err)
		print_fs_err(err);
}
static void run_mount() {
	if (!fs_init()) return;
	// mount the filesystem
	int err = lfs_mount(&lfs, &cfg);
	if (LFS_ERR_OK != err)
		print_fs_err(err);
}
static void run_unmount() {
	int err = lfs_unmount(&lfs);
	if (LFS_ERR_OK != err)
		print_fs_err(err);
}
static void run_getfree() {
	/* Get total sectors and free sectors */
	lfs_ssize_t allocated = lfs_fs_size(&lfs);
	if (allocated < 0) {
		print_fs_err(allocated);
		return;
	}
	lfs_ssize_t free = cfg.block_count - allocated;
	printf("Free blocks: %lu (%3.1f%% full)\r\n", free,
			100.0f * allocated / cfg.block_count);
}
static void run_mkdir() {
	char *arg1 = strtok(NULL, " ");
	if (!arg1) {
		printf("Missing argument\r\n");
		return;
	}
	int err = lfs_mkdir(&lfs, arg1);
	if (LFS_ERR_OK != err)
		print_fs_err(err);
}
void run_ls() {
	char *arg1 = strtok(NULL, " ");
	if (!arg1) {
		arg1 = "/";
	}
	lfs_dir_t dir = {0};
//	memset(&dir, 0, sizeof dir);

	// Once open a directory can be used with read to iterate over files.
	// Returns a negative error code on failure.
	int err = lfs_dir_open(&lfs, &dir, arg1);
	if (LFS_ERR_OK != err) {
		print_fs_err(err);
		return;
	}
	printf("Directory Listing:\r\n");

	// Read an entry in the directory
	//
	// Fills out the info structure, based on the specified file or directory.
	// Returns a positive value on success, 0 at the end of directory,
	// or a negative error code on failure.
	int rc;
	do {
		struct lfs_info info;
		memset(&info, 0, sizeof info);
		rc = lfs_dir_read(&lfs, &dir, &info);
		if (!rc)
			break;
		if (rc < 0) {
			print_fs_err(err);
			break;
		}
		if (LFS_TYPE_DIR == info.type) {
			printf("\t%lu\t[DIR]\t%s\r\n", info.size, info.name);
		} else {
			printf("\t%lu\t%s\r\n", info.size, info.name);
		}
	} while (rc > 0);

	// Close a directory
	//
	// Releases any allocated resources.
	// Returns a negative error code on failure.
	err = lfs_dir_close(&lfs, &dir);
	if (LFS_ERR_OK != err) {
		print_fs_err(err);
		return;
	}
}
static void run_cat() {
	char *arg1 = strtok(NULL, " ");
	if (!arg1) {
		printf("Missing argument\r\n");
		return;
	}
	lfs_file_t file;
	memset(&file, 0, sizeof file);
	int err = lfs_file_open(&lfs, &file, arg1, LFS_O_RDONLY);
	if (LFS_ERR_OK != err) {
		print_fs_err(err);
		return;
	}
	// Read data from file
	//
	// Takes a buffer and size indicating where to store the read data.
	// Returns the number of bytes read, or a negative error code on failure.
	char buf[128];
	lfs_ssize_t sz;
	do {
		sz = lfs_file_read(&lfs, &file, buf, sizeof buf);
		if (sz > 0)
			printf("%.*s", sz, buf);
	} while (sz > 0);
	if (sz < 0)
		print_fs_err(err);

	err = lfs_file_close(&lfs, &file);
	if (LFS_ERR_OK != err)
		print_fs_err(err);
}
static void run_help();

typedef void (*p_fn_t)();
typedef struct {
	char const *const command;
	p_fn_t const function;
	char const *const help;
} cmd_def_t;

static cmd_def_t cmds[] =
{
	{ "setrtc", run_setrtc, "setrtc <DD> <MM> <YY> <hh> <mm> <ss>:\r\n"
		"  Set Real Time Clock\r\n"
		"  Parameters: new date (DD MM YY) new time in 24-hour format "
		"(hh mm ss)\r\n"
		"\te.g.:setrtc 16 3 21 0 4 0" },
	{ "date", run_date, "date:\r\n Print current date and time" },
	{ "lliot", run_lliot,
		"lliot:\r\n !DESTRUCTIVE! Low Level I/O Driver Test" },
	{ "simple", run_simple, "simple:\r\n  Run simple FS tests" },
	{ "format", run_format,
		"format:\r\n Creates a volume on the SD card." },
	{ "mount", run_mount,
		"mount:\r\n Register the work area of the volume" },
	{ "unmount", run_unmount,
		"unmount:\r\n Unregister the work area of the volume" },
	{ "getfree", run_getfree,
			"getfree:\r\n Print the free space on drive" },
	{ "mkdir", run_mkdir, "mkdir <path>:\r\n"
		"  Make a new directory.\r\n"
		"  <path> Specifies the name of the directory to be created.\r\n"
		"\te.g.: mkdir /dir1" },
	{ "ls", run_ls, "ls <path>:\r\n  List directory" },
	{ "cat", run_cat, "cat <filename>:\r\n  Type file contents" },
	{ "help", run_help,
		"help:\r\n"
		"  Shows this command help." }
};
static void run_help() {
	for (size_t i = 0; i < count_of(cmds); ++i) {
		printf("%s\n\r\n", cmds[i].help);
	}
}

void process_command(char *cmd) {
	/* Process the input string received prior to the newline. */
	char *cmdn = strtok(cmd, " ");
	if (cmdn) {
		size_t i;
		for (i = 0; i < count_of(cmds); ++i) {
			if (0 == strcmp(cmds[i].command, cmdn)) {
				(*cmds[i].function)();
				break;
			}
		}
		if (count_of(cmds) == i)
			printf("Command \"%s\" not found\r\n", cmdn);
	}
}
