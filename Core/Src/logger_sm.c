/*
 * logger.c
 *
 *  Created on: Jan 14, 2022
 *      Author: carlk
 */

#include <time.h>
//
#include "freertos.h"
#include "data.h"
#include "fs.h"
//
#include "logger_sm.h"
//
#include "printf.h"

typedef struct log_t log_t;
typedef void (*log_state_t)(evt_t const* const);
struct log_t {
	log_state_t state;
};
static evt_t log_entry_evt = { LOG_ENTRY_SIG, { 0 } };
static evt_t log_exit_evt = { LOG_EXIT_SIG, { 0 } };

static void log_opening_st(evt_t const *const pEvt);  // fwd decl
static void log_run_st(evt_t const *const pEvt);
static void log_failed_st(evt_t const *const pEvt);
static void log_idle_st(evt_t const *const pEvt);

static log_t log = { log_opening_st }; // the state machine instance

void log_dispatch(evt_t const *evt) {
	(log.state)(evt);
}
static void log_tran(log_state_t target) {
	log_dispatch(&log_exit_evt); /* exit the source */
	log.state = target;
	log_dispatch(&log_entry_evt); /* enter the target */
}

static void log_failed_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case LOG_ENTRY_SIG:
	case LOG_STOP_SIG: {
		uint32_t flags = osEventFlagsSet(TaskStoppedHandle, TASK_LOG);
		configASSERT(!(0x80000000 & flags));
		printf("Logger failed\r\n");
		break;
	}
	default:
		;
	}
}
static void log_idle_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case LOG_ENTRY_SIG: {
		uint32_t flags = osEventFlagsSet(TaskStoppedHandle, TASK_LOG);
		configASSERT(!(0x80000000 & flags));
		printf("Logger idle\r\n");
		break;
	}
	case LOG_START_SIG:
		log_tran(log_run_st);
		break;
	case LOG_STOP_SIG: {
		uint32_t flags = osEventFlagsSet(TaskStoppedHandle, TASK_LOG);
		configASSERT(!(0x80000000 & flags));
		break;
	}
	default:
		;
	}
}

static lfs_file_t msg_file, data_file;

static bool open_msg_file() {
//		memset(&msg_file, 0, sizeof msg_file);
	int err = lfs_file_open(&lfs, &msg_file, "log.txt",
			LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
	if (LFS_ERR_OK != err) {
		print_fs_err(err);
		log_tran(log_failed_st);
		return false;
	}
	lfs_ssize_t nfw = lfs_file_write(&lfs, &msg_file, "\n", 1);
	if (nfw < 0) {
		print_fs_err(nfw);
		log_tran(log_failed_st);
		return false;
	} else {
		configASSERT(1 == nfw);
	}
	return true;
}
static bool open_data_file() {
	char pathname[128] = {0};
	size_t n = snprintf(pathname, sizeof pathname, "/data");
	lfs_mkdir(&lfs, pathname);
	struct tm tmbuf;
	const time_t secs = time(NULL);
	localtime_r(&secs, &tmbuf);
	//  tm_year	int	years since 1900
	//  tm_mon	int	months since January	0-11
	//  tm_mday	int	day of the month	1-31
	//  tm_hour	int	hours since midnight	0-23
	n += snprintf(pathname + n, sizeof pathname - n, "/%d", tmbuf.tm_year + 1900);
	lfs_mkdir(&lfs, pathname);
	n += snprintf(pathname + n, sizeof pathname - n, "/%02d", tmbuf.tm_mon + 1);
	lfs_mkdir(&lfs, pathname);
	n += snprintf(pathname + n, sizeof pathname - n, "/%02d.csv", tmbuf.tm_mday);
	configASSERT(n < sizeof pathname);
	int err = lfs_file_open(&lfs, &data_file, pathname,
			LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
	if (LFS_ERR_OK != err) {
		print_fs_err(err);
		log_tran(log_failed_st);
		return false;
	}
	const char *s = "\nDate,Time,RPM,B+ Volts,B+ Amps,Internal °C,ADC11 °C\n";
	lfs_ssize_t nfw = lfs_file_write(&lfs, &data_file, s, strlen(s));
	if (nfw < 0) {
		print_fs_err(nfw);
		log_tran(log_failed_st);
		return false;
	} else {
		configASSERT((lfs_ssize_t)strlen(s) == nfw);
	}
	return true;
}
static void log_opening_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case LOG_ENTRY_SIG:
		if (!fs_init()) {
			log_tran(log_failed_st);
			break;
		}
		// mount the filesystem
		int err = lfs_mount(&lfs, &cfg);
		if (LFS_ERR_OK != err)
			print_fs_err(err);
		// reformat if we can't mount the filesystem
		// this should only happen on the first boot
		if (err) {
			err = lfs_format(&lfs, &cfg);
			if (LFS_ERR_OK != err)
				print_fs_err(err);
			err = lfs_mount(&lfs, &cfg);
			if (LFS_ERR_OK != err) {
				print_fs_err(err);
				log_tran(log_failed_st);
				break;
			}
		}
		/* Open the message file */
		if (!open_msg_file()) {
			log_tran(log_failed_st);
			break;
		}
		/* Open data log file */
		if (!open_data_file()) {
			log_tran(log_failed_st);
			break;
		}
		log_tran(log_run_st);
		break;
	default:
		;
	}
}
static void log_run_st(evt_t const *const pEvt) {
	static bool line_begin;
	switch (pEvt->sig) {
	case LOG_ENTRY_SIG:
		line_begin = true;
		break;
	case LOG_PUTCH_SIG: {
		char c = pEvt->content.data;
		lfs_ssize_t nfw;
		if (line_begin) {
			char lbuf[32] = {0};
			const time_t secs = time(NULL);
			struct tm tmbuf;
			struct tm *ptm = localtime_r(&secs, &tmbuf);
			size_t n = strftime(lbuf, sizeof lbuf, "%F,%T,", ptm);
			configASSERT(n);
			nfw = lfs_file_write(&lfs, &msg_file, lbuf, n);
			if (nfw < 0) {
				print_fs_err(nfw);
				log_tran(log_failed_st);
				break;
			} else {
				configASSERT(nfw == (int )n);
			}
			line_begin = false;
		}
		nfw = lfs_file_write(&lfs, &msg_file, &c, 1);
		if (nfw < 0) {
			print_fs_err(nfw);
			log_tran(log_failed_st);
			break;
		} else {
			configASSERT(1 == nfw);
		}
		if ('\n' == c) {
			int err = lfs_file_sync(&lfs, &msg_file);
			if (LFS_ERR_OK != err) {
				print_fs_err(err);
				log_tran(log_failed_st);
				break;
			}
			line_begin = true;
		}
		break;
	}
	case PERIOD_1HZ_SIG: {
		char lbuf[128] = {0};
		const time_t secs = time(NULL);
		struct tm tmbuf;
		struct tm *ptm = localtime_r(&secs, &tmbuf);
		size_t n = strftime(lbuf, sizeof lbuf, "%F,%T,", ptm);
		configASSERT(n);
		data_rec_t data = {0};
		get_data_1sec_avg(&data);
		int m = snprintf_(lbuf + n, sizeof lbuf - n, "%.0f,%.2f,%.2f,%.1f,%.1f\n",
				data.rpm, data.Bvolts, data.Bamps, data.internal_temp, data.ADC11_degC);
		configASSERT(m > 0);
		n += m;
		configASSERT(n < sizeof(lbuf));
		lfs_ssize_t nfw = lfs_file_write(&lfs, &data_file, lbuf, n);
		if (nfw < 0) {
			print_fs_err(nfw);
			log_tran(log_failed_st);
			break;
		} else {
			configASSERT(nfw == (int )n);
		}
		int err = lfs_file_sync(&lfs, &data_file);
		if (LFS_ERR_OK != err) {
			print_fs_err(err);
			log_tran(log_failed_st);
			break;
		}
		break;
	}
	case LOG_STOP_SIG:
		log_tran(log_idle_st);
		break;
	case LOG_EXIT_SIG: {
		int err = lfs_file_close(&lfs, &data_file);
		if (LFS_ERR_OK != err) {
			print_fs_err(err);
			break;
		}
		err = lfs_file_close(&lfs, &msg_file);
		if (LFS_ERR_OK != err) {
			print_fs_err(err);
			break;
		}
		err = lfs_unmount(&lfs);
		if (LFS_ERR_OK != err) {
			print_fs_err(err);
			break;
		}
	}
	default:
		;
	}
}

// define the output function
static void log_putch(char character, void *arg) {
	// opt. evaluate the argument and send the char somewhere
	(void) arg;
	evt_t evt = { LOG_PUTCH_SIG, { .data = character } };
	osStatus_t rc = osMessageQueuePut(LoggerEvtQHandle, &evt, 0, 100);
	configASSERT(osOK == rc);
}

void log_printf(const char *const pcFormat, ...) {
	va_list xArgs;
	va_start(xArgs, pcFormat);
	printf(pcFormat, xArgs);
	fctprintf(log_putch, 0, pcFormat, xArgs);
	va_end(xArgs);
}
