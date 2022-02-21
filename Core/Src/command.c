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

static const char *missing_arg_str = "Missing argument; see help\n";

static void run_setrtc() {
	const char *dateStr = strtok(NULL, " ");
	if (!dateStr) {
		printf(missing_arg_str);
		return;
	}
	int date = atoi(dateStr);

	const char *monthStr = strtok(NULL, " ");
	if (!monthStr) {
		printf(missing_arg_str);
		return;
	}
	int month = atoi(monthStr);

	const char *yearStr = strtok(NULL, " ");
	if (!yearStr) {
		printf(missing_arg_str);
		return;
	}
	int year = atoi(yearStr);

	const char *hourStr = strtok(NULL, " ");
	if (!hourStr) {
		printf(missing_arg_str);
		return;
	}
	int hour = atoi(hourStr);

	const char *minStr = strtok(NULL, " ");
	if (!minStr) {
		printf(missing_arg_str);
		return;
	};
	int min = atoi(minStr);

	const char *secStr = strtok(NULL, " ");
	if (!secStr) {
		printf(missing_arg_str);
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
static void run_big_file_test() {
    const char *pcPathName = strtok(NULL, " ");
    if (!pcPathName) {
        printf(missing_arg_str);
        return;
    }
    const char *pcSize = strtok(NULL, " ");
    if (!pcSize) {
        printf(missing_arg_str);
        return;
    }
    size_t size = strtoul(pcSize, 0, 0);
    const char *pcSeed = strtok(NULL, " ");
    if (!pcSeed) {
        printf(missing_arg_str);
        return;
    }
    uint32_t seed = atoi(pcSeed);
    void big_file_test(const char *const pathname, size_t size, uint32_t seed); // See Core/sd_driver/big_file_test.c
    big_file_test(pcPathName, size, seed);
}
static void run_date() {
	char buf[128] = { 0 };
	time_t epoch_secs = time(NULL);
	struct tm *ptm = localtime(&epoch_secs);
	size_t n = strftime(buf, sizeof(buf), "%c", ptm);
	myASSERT(n);
	printf("%s\n", buf);
	strftime(buf, sizeof(buf), "%j", ptm); // The day of the year as a decimal number (range
										   // 001 to 366).
	printf("Day of year: %s\n", buf);
}

static void run_format() {
	if (!fs_init()) return;
	/* Format the drive with default parameters */
	int err = lfs_format(&lfs, &cfg);
	if (LFS_ERR_OK != err)
		print_fs_err("format", err);
}
static void run_mount() {
	if (!fs_init()) return;
	// mount the filesystem
	int err = lfs_mount(&lfs, &cfg);
	if (LFS_ERR_OK != err)
		print_fs_err("mount", err);
}
static void run_unmount() {
	int err = lfs_unmount(&lfs);
	if (LFS_ERR_OK != err)
		print_fs_err("unmount", err);
}
static void run_getfree() {
	/* Get total sectors and free sectors */
	lfs_ssize_t allocated = lfs_fs_size(&lfs);
	if (allocated < 0) {
		print_fs_err("lfs_fs_size", allocated);
		return;
	}
	lfs_ssize_t free = cfg.block_count - allocated;
	printf_("Free blocks: %lu (%3.1f%% full)\n", free,
			100.0f * allocated / cfg.block_count);
}
static void run_mkdir() {
	char *arg1 = strtok(NULL, " ");
	if (!arg1) {
		printf(missing_arg_str);
		return;
	}
	int err = lfs_mkdir(&lfs, arg1);
	if (LFS_ERR_OK != err)
		print_fs_err("mkdir", err);
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
		print_fs_err("dir_open", err);
		return;
	}
	printf("Directory Listing:\n");

	// Read an entry in the directory
	//
	// Fills out the info structure, based on the specified file or directory.
	// Returns a positive value on success, 0 at the end of directory,
	// or a negative error code on failure.
	int rc;
	do {
		struct lfs_info info = {0};
		rc = lfs_dir_read(&lfs, &dir, &info);
		if (!rc)
			break;
		if (rc < 0) {
			print_fs_err("read", err);
			break;
		}
		if (LFS_TYPE_DIR == info.type) {
			printf("\t%lu\t[DIR]\t%s\n", info.size, info.name);
		} else {
			printf("\t%lu\t%s\n", info.size, info.name);
		}
	} while (rc > 0);

	// Close a directory
	//
	// Releases any allocated resources.
	// Returns a negative error code on failure.
	err = lfs_dir_close(&lfs, &dir);
	if (LFS_ERR_OK != err) {
		print_fs_err("close", err);
		return;
	}
}
static void run_cat() {
	char *arg1 = strtok(NULL, " ");
	if (!arg1) {
		printf(missing_arg_str);
		return;
	}
	lfs_file_t file = {0};
	int err = lfs_file_opencfg(&lfs, &file, arg1, LFS_O_RDONLY, &file_cfg);
	if (LFS_ERR_OK != err) {
		print_fs_err("lfs_file_opencfg", err);
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
		print_fs_err("read", err);

	err = lfs_file_close(&lfs, &file);
	if (LFS_ERR_OK != err)
		print_fs_err("close", err);
}
void run_rm() {
	char *arg1 = strtok(NULL, " ");
	if (!arg1) {
		printf(missing_arg_str);
		return;
	}
	int err = lfs_remove(&lfs, arg1);
	if (LFS_ERR_OK != err)
		print_fs_err("lfs_remove", err);
}
void run_task_stats() {
	printf(
        "Task          State  Priority  Stack	"
        "#\n************************************************\n");
    /* NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
	char buf[256] = {0};
	buf[255] = 0xA5;  // Crude overflow guard
    /* Generate a table of task stats. */
    vTaskList(buf);
    configASSERT(0xA5 == buf[255]);
    printf("%s\n", buf);
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
	{ "setrtc", run_setrtc, "setrtc <DD> <MM> <YY> <hh> <mm> <ss>:\n"
		"  Set Real Time Clock\n"
		"  Parameters: new date (DD MM YY) new time in 24-hour format "
		"(hh mm ss)\n"
		"\te.g.:setrtc 16 3 21 0 4 0" },
	{ "date", run_date, "date:\n Print current date and time" },
	{ "lliot", run_lliot,
		"lliot:\n !DESTRUCTIVE! Low Level I/O Driver Test" },
	{ "simple", run_simple, "simple:\n  Run simple FS tests" },
    {"bft", run_big_file_test,
     "bft <pathname> <size in bytes> <seed>:\n"
	 " Big File Test\n"
     " Writes random data to file <pathname>.\n"
     " <size in bytes> must be multiple of 512.\n"
     "\te.g.: bft bf 1048576 1\n"
     "\tor: bft big1G-1 0x40000000 1"},
	{ "format", run_format,
		"format:\n Creates a volume on the SD card." },
	{ "mount", run_mount,
		"mount:\n Register the work area of the volume" },
	{ "unmount", run_unmount,
		"unmount:\n Unregister the work area of the volume" },
	{ "getfree", run_getfree,
			"getfree:\n Print the free space on drive" },
	{ "mkdir", run_mkdir, "mkdir <path>:\n"
		"  Make a new directory.\n"
		"  <path> Specifies the name of the directory to be created.\n"
		"\te.g.: mkdir /dir1" },
	{ "ls", run_ls, "ls <path>:\n  List directory" },
	{ "cat", run_cat, "cat <pathname>:\n  Type file contents" },
	{ "rm", run_rm, "rm <pathname>:\n Remove file" },
	{ "task-stats", run_task_stats, "Show task statistics" },
	{ "help", run_help,
		"help:\n"
		"  Shows this command help." }
};
static void run_help() {
	for (size_t i = 0; i < count_of(cmds); ++i) {
		printf("%s\n\n", cmds[i].help);
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
			printf("Command \"%s\" not found\n", cmdn);
	}
}
