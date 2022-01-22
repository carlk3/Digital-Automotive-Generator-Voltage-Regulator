/*
 * logger.c
 *
 *  Created on: Jan 14, 2022
 *      Author: carlk
 */

#include <time.h>
//
#include "logger_sm.h"
#include "freertos.h"
#include "fs.h"
//
#include "printf.h"

typedef struct log_t log_t;
typedef void (*log_state_t)(evt_t const* const);
struct log_t {
	log_state_t state;
};
static evt_t log_entry_evt = { LOG_ENTRY_SIG, { 0 } };
static evt_t log_exit_evt = { LOG_EXIT_SIG, { 0 } };

static void log_run_st(evt_t const *const pEvt);  // fwd decl
static void log_failed_st(evt_t const *const pEvt);
static void log_idle_st(evt_t const *const pEvt);

static log_t log = { log_run_st }; // the state machine instance

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
		printf("Logger failed\r\n");
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
	default:
		;
	}
}

static lfs_file_t file;

static void log_run_st(evt_t const *const pEvt) {
	static bool line_begin;
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
		memset(&file, 0, sizeof file);
		err = lfs_file_open(&lfs, &file, "log.txt",
				LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
		if (LFS_ERR_OK != err) {
			print_fs_err(err);
			log_tran(log_failed_st);
			break;
		}
		lfs_ssize_t nfw = lfs_file_write(&lfs, &file, "\n", 1);
		if (nfw < 0) {
			print_fs_err(nfw);
			log_tran(log_failed_st);
			break;
		} else {
			configASSERT(1 == nfw);
		}
		line_begin = true;
		break;
	case LOG_PUTCH_SIG: {
		char c = pEvt->content.data;
		lfs_ssize_t nfw;
		if (line_begin) {
			char lbuf[32];
			const time_t secs = time(NULL);
			struct tm tmbuf;
			struct tm *ptm = localtime_r(&secs, &tmbuf);
			size_t n = strftime(lbuf, sizeof lbuf, "%F,%T,", ptm);
			configASSERT(n);
			nfw = lfs_file_write(&lfs, &file, lbuf, n);
			if (nfw < 0) {
				print_fs_err(nfw);
				log_tran(log_failed_st);
				break;
			} else {
				configASSERT(nfw == (int )n);
			}
			line_begin = false;
		}
		nfw = lfs_file_write(&lfs, &file, &c, 1);
		if (nfw < 0) {
			print_fs_err(nfw);
			log_tran(log_failed_st);
			break;
		} else {
			configASSERT(1 == nfw);
		}
		if ('\n' == c) {
			int err = lfs_file_sync(&lfs, &file);
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
		break;
	}
	case LOG_STOP_SIG:
		log_tran(log_idle_st);
		break;
	case LOG_EXIT_SIG: {
		int err = lfs_file_close(&lfs, &file);
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
