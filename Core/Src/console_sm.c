#include <ctype.h> // tolower
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>     /* strtoul */

#include "main.h"
#include "freertos.h"
//
#include "analog.h"
#include "command.h"
#include "config.h"
#include "data.h"
#include "digital.h"
#include "fs.h"
#include "tim.h"
//
#include "printf.h"

typedef struct cnsl_t cnsl_t;
typedef void (*cnsl_state_t)(evt_t const* const);
struct cnsl_t {
	cnsl_state_t state;
	char buf[128];
	size_t buf_end;
};
static evt_t cnsl_entry_evt = { CNSL_ENTRY_SIG, { 0 } };
static evt_t cnsl_exit_evt = { CNSL_EXIT_SIG, { 0 } };

static void cnsl_top_st(evt_t const *const pEvt);  // fwd decl
static void cnsl_show_st(evt_t const *const pEvt);
static void cnsl_stat_st(evt_t const *const pEvt);
static void cnsl_cfg_st(evt_t const *const pEvt);
static void cnsl_cfg_v_st(evt_t const *const pEvt);
static void cnsl_cfg_c_st(evt_t const *const pEvt);
static void cnsl_cfg_p_st(evt_t const *const pEvt);
static void cnsl_cal_st(evt_t const *const pEvt);
static void cnsl_cal_v_st(evt_t const *const pEvt);
static void cnsl_cal_z_st(evt_t const *const pEvt);
static void cnsl_cal_c_st(evt_t const *const pEvt);
static void cnsl_cmd_st(evt_t const *const pEvt);

static cnsl_t cnsl = {		// the state machine instance
		cnsl_top_st, "", 0 };
void cnsl_dispatch(evt_t const *evt) {
	(cnsl.state)(evt);
}
static void cnsl_tran(cnsl_state_t target) {
	cnsl_dispatch(&cnsl_exit_evt); /* exit the source */
	cnsl.state = target;
	cnsl_dispatch(&cnsl_entry_evt); /* enter the target */
}


static void cnsl_top_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("Top Menu\n"
				"S: Show Continuous Readings\n"
				"T: Show Statistics\n"
				"O: Configuration\n"
				"C: Calibration\n"
				"L: Command Line\n"
				"Enter a letter: ");
		fflush(stdout);
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case 's':
			printf_("\n");
			cnsl_tran(cnsl_show_st);
			break;
		case 't':
			printf_("\n");
			cnsl_tran(cnsl_stat_st);
			break;
		case 'o':
			printf_("\n");
			cnsl_tran(cnsl_cfg_st);
			break;
		case 'c':
			printf_("\n");
			cnsl_tran(cnsl_cal_st);
			break;
		case 'l':
			printf_("\n");
			cnsl_tran(cnsl_cmd_st);
			break;

		case '1': {
			printf_("\n");
//			enable_33_pwr(true);
			evt_t evt = { REG_START_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case '2': {
			printf_("\n");
//			enable_33_pwr(false);
			evt_t evt = { REG_STOP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case '3':
			printf_("\n");
			enable_field(true);
			printf_("field enabled\n");
			break;
		case '4':
			printf_("\n");
			enable_field(false);
			printf_("field disabled\n");
			break;
		case '5': {
			evt_t evt = { REG_SLEEP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case '6': {
			printf_("\n");
			evt_t evt = { LOG_STOP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(LoggerEvtQHandle, &evt, 0, 3000);
			if (osOK != rc) {
				printf_("Dropped LOG_STOP_SIG to LoggerEvtQ\n");
			} else {
				uint32_t flags = osEventFlagsWait(TaskStoppedHandle, TASK_LOG, osFlagsWaitAll, 3000);
				configASSERT(!(0x80000000 & flags));
			}
			break;
		}
		default:
			printf_("\n");
			cnsl_dispatch(&cnsl_entry_evt);
		} //
	}
	default:
		;
	}
}
static void cnsl_show_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("Show continuous readings\n"
				"[Enter anything to quit]\n"
				"RPM    Volts  Amps   Int °C A11 °C\n"
				"------ ------ ------ ------ ------\n");
		break;
	case KEYSTROKE_SIG:
		if (isprint(pEvt->content.data)) {
			printf_("\n");
			cnsl_tran(cnsl_top_st);
		}
		break;
	case PERIOD_1HZ_SIG: {
		data_rec_t data;
//		get_data_1sec_avg(&data);
		get_data(&data);
		printf_("%6.0f %6.2f %6.2f %6.1f %6.1f\n", data.rpm, data.Bvolts,
				data.Bamps, data.internal_temp, data.ADC11_degC);
		break;
	}
	default:
		;
	}
}
static void cnsl_stat_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("\r\nUptime: %llu seconds\n", uptime);
		printf_("Internal Temperature:\r\n\t"
				"Mean: %4.3g\r\n\t"
//				 "StdDev: %#7.3g\r\n\t"
				"Min: %6.4g\r\n\t"
				"Max: %6.4g\n",
				RS_Mean(&internal_temp_stats),
//				 RS_StandardDeviation(&internal_temp_stats),
				RS_Min(&internal_temp_stats),
				RS_Max(&internal_temp_stats));
		printf_("B+ Voltage: \r\n\t"
				"Mean: %4.4g\r\n\t"
				"Min: %4.3g\r\n\t"
				"Max: %4.3g\n",
				RS_Mean(&Bplus_volt_stats),
				RS_Min(&Bplus_volt_stats),
				RS_Max(&Bplus_volt_stats));
		printf_("B+ Current:\r\n\t"
				"Mean: %6.4g\r\n\t"
				"Min: %6.3g\r\n\t"
				"Max: %6.3g\n",
				RS_Mean(&Bplus_amp_stats),
				RS_Min(&Bplus_amp_stats),
				RS_Max(&Bplus_amp_stats));
		printf_("\r\nEnter:\n"
				"T: Show again\n"
				"R: Reset statistics\n"
				"[anything else]: Quit\n");
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case 't':
			cnsl_dispatch(&cnsl_entry_evt);
			break;
		case 'r':
			reset_stats();
			cnsl_dispatch(&cnsl_entry_evt);
			break;
		default:
			if (isprint(pEvt->content.data)) {
				printf_("\n");
				cnsl_tran(cnsl_top_st);
			}
			break;
		} // switch (tolower(c))
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
} // static void cnsl_stat_st

static void cnsl_cfg_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("Configuration\n"
				"Settings:\n"
		"  V: Voltage limit: %6.3g\n"
		"  C: Current limit: %6.3g\n"
		"  P: Power limit: %6.3g\n"
		"  S: Save\n"
		"Enter V, C, or P to change,\n"
		"S to commit changes: ",
		cfg_get_vlim(),
		cfg_get_clim(),
		cfg_get_plim());
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		if (!isprint(c)) break;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case 'v':
			cnsl_tran(cnsl_cfg_v_st);
			break;
		case 'c':
			cnsl_tran(cnsl_cfg_c_st);
			break;
		case 'p':
			cnsl_tran(cnsl_cfg_p_st);
			break;
		case 's':
			cfg_save();
			printf_("\r\nConfiguration saved\n");
			break;
		default:
			printf_("\n");
			cnsl_tran(cnsl_top_st);
			break;
		} // switch (tolower(c))
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cfg_v_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("\nConfigure Voltage Limit\n"
				"[Enter new limit in volts,\n"
				"or anything else to quit]\n");
		printf_("Existing: %6.3g\n", cfg_get_vlim());
		printf_("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end < sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || '\n' == c) {
			float f;
			int rc = sscanf(cnsl.buf, "%f", &f);
			if (1 == rc)
				cfg_set_vlim(f);
			cnsl_dispatch(&cnsl_entry_evt);
		} else {
			cnsl_tran(cnsl_cfg_st);
		}
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cfg_c_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("\nConfigure Current Limit\n"
				"[Enter new limit in amps,\n"
				"or anything else to quit]\n");
		printf_("Existing: %6.3g\n", cfg_get_clim());
		printf_("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end < sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || '\n' == c) {
			float f;
			int rc = sscanf(cnsl.buf, "%f", &f);
			if (1 == rc)
				cfg_set_clim(f);
			cnsl_dispatch(&cnsl_entry_evt);
		} else {
			cnsl_tran(cnsl_cfg_st);
		}
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cfg_p_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf_("Configure Power Limit\n"
				"[Enter new limit in watts,\n"
				"or anything else to quit]\n");
		printf_("Existing: %6.3g\n", cfg_get_plim());
		printf_("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end < sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || '\n' == c) {
			float f;
			int rc = sscanf(cnsl.buf, "%f", &f);
			if (1 == rc)
				cfg_set_plim(f);
			cnsl_dispatch(&cnsl_entry_evt);
		} else {
			cnsl_tran(cnsl_cfg_st);
		}
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}

static void cnsl_cal_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG: {
		data_rec_t data;
		get_data_1sec_avg(&data);
		printf_("\nCalibration\n"
		"  V: Voltage sense\n"
		"  Z: Current sense zero\n"
		"  C: Current sense scale\n"
		"Enter V, Z, or C to calibrate,\n"
		"or anything else to quit: ");
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		if (!isprint(c)) break;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case 'v':
			cnsl_tran(cnsl_cal_v_st);
			break;
		case 'z':
			cnsl_tran(cnsl_cal_z_st);
			break;
		case 'c':
			cnsl_tran(cnsl_cal_c_st);
			break;
		default:
			printf_("\n");
			cnsl_tran(cnsl_top_st);
			break;
		} // switch (tolower(c))
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cal_v_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG: {
		data_rec_t data;
		get_data_1sec_avg(&data);
		printf_("\nCalibrate Voltage Sense\n"
				"[Enter true voltage, \n"
				"or anything else to quit] \n");
		printf_("Sensed: %6.3g\n", data.Bvolts);
		printf_("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end < sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || '\n' == c) {
			float truth;
			int rc = sscanf(cnsl.buf, "%f", &truth);
			if (1 == rc) {
				data_rec_t data;
				get_data_1sec_avg(&data);
				cfg_set_Bplus_volt_scale(truth / data.Bvolts_raw);
				cfg_save();
			}
			cnsl_dispatch(&cnsl_entry_evt);
		} else {
			cnsl_tran(cnsl_cal_st);
		}
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cal_z_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG: {
		data_rec_t data;
		get_data_1sec_avg(&data);
		printf_("\nCalibrate Current Sense Zero\n"
				"[Disconnect B+ and press C to continue,\n"
				"or anything else to quit] \n");
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('c' == tolower(c)) {
			data_rec_t data;
			get_data_1sec_avg(&data);
			uint16_t raw_zero = data.Bamps_raw;
			cfg_set_Bplus_amp_zero(raw_zero);
			cfg_save();
			printf("\nSaved\n");
		}
		cnsl_tran(cnsl_cal_st);
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cal_c_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG: {
		data_rec_t data;
		get_data_1sec_avg(&data);
		printf_("\nCalibrate Current Sense\n"
				"[Connect ammeter inline to B+, and enter true current, \n"
				"or anything else to quit] \n");
		printf_("Enter amps: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || '-' == c || isdigit(c)) {
			if (cnsl.buf_end < sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || '\n' == c) {
			float truth;
			int rc = sscanf(cnsl.buf, "%f", &truth);
			if (1 == rc) {
//				if (fabs(*pOldV - v) >= FLT_EPSILON * fmaxf(fabs(*pOldV), fabs(v)))
				if (truth < 1.0) {
					printf_("Error: current too low for calibration\n");
					cnsl_dispatch(&cnsl_entry_evt);
					break;
				}
				data_rec_t data;
				get_data_1sec_avg(&data);
				uint16_t raw_amps = data.Bamps_raw;
				uint16_t raw_zero = cfg_get_Bplus_amp_zero();
				cfg_set_Bplus_amp_scale(truth / (raw_amps - raw_zero));
				cfg_save();
				printf("\nSaved\n");
			}
			cnsl_tran(cnsl_cal_st);
		} else {
			cnsl_tran(cnsl_cal_st);
		}
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
static void cnsl_cmd_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG: {
		printf_("Command Line\n"
				"[Enter \"exit\" to quit] \n"
				"> ");
		fflush(stdout);
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		printf_("%c", c);  // echo
		fflush(stdout);
		if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || '\n' == c) { // enter
			if (0 == strcmp("exit", cnsl.buf)) {
				cnsl_tran(cnsl_top_st);
			} else {
				printf_("\n");
				process_command(cnsl.buf);
				memset(cnsl.buf, 0, sizeof cnsl.buf);
				cnsl.buf_end = 0;
				printf_("> ");
				fflush(stdout);
			}
		} else {
			if (cnsl.buf_end < sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		}
		break;
	} // case KEYSTROKE_SIG
	default:
		;
	} // switch (pEvt->sig)
}
