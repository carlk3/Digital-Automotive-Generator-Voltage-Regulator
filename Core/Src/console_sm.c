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

static void cnsl_show_st(evt_t const *const pEvt);
static void cnsl_stat_st(evt_t const *const pEvt);
static void cnsl_cfg_st(evt_t const *const pEvt);
static void cnsl_cfg_v_st(evt_t const *const pEvt);
static void cnsl_cfg_c_st(evt_t const *const pEvt);
static void cnsl_cfg_p_st(evt_t const *const pEvt);
static void cnsl_cal_st(evt_t const *const pEvt);
static void cnsl_cal_v_st(evt_t const *const pEvt);
static void cnsl_cal_c_st(evt_t const *const pEvt);

static void cnsl_top_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf("Top Menu\r\n"
				"S: Show Continuous Readings\r\n"
				"T: Show Statistics\r\n"
				"O: Configuration\r\n"
				"C: Callibration\r\n"
				"L: Command Line\r\n"
				"Enter a letter: ");

//				"r: Reset statistics\r\n"
//				"2: Calibrate\r\n"
//				"3: Configure\r\n"
//				"4 or v: Show B+ voltage\r\n"
//				"5 or c: Show B+ current\r\n"
//				"t: show RPM\r\n"
//				"e: Show ADC11 as temperature\r\n"
//				"s: start regulator\r\n"
//				"q: stop regulator\r\n"
//				"f: enable field\r\n"
//				"g: disable field\r\n"
//				"l: sleep\r\n"
//				"d: test SD card\r\n"
//				"i: littlefs test\r\n"
//				"[Type choice]: ");
		fflush(stdout);
//		memset(cnsl.buf, 0, sizeof cnsl.buf);
//		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case 's':
			printf("\r\n");
			cnsl_tran(cnsl_show_st);
			break;
		case 't':
			printf("\r\n");
			cnsl_tran(cnsl_stat_st);
			break;
		case 'o':
			printf("\r\n");
			cnsl_tran(cnsl_cfg_st);
			break;
		case 'c':
			printf("\r\n");
			cnsl_tran(cnsl_cal_st);
			break;

		case '1': {
			printf("\r\n");
//			enable_33_pwr(true);
			evt_t evt = { REG_START_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case '2': {
			printf("\r\n");
//			enable_33_pwr(false);
			evt_t evt = { REG_STOP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case '3':
			printf("\r\n");
			enable_field(true);
			printf("field enabled\r\n");
			break;
		case '4':
			printf("\r\n");
			enable_field(false);
			printf("field disabled\r\n");
			break;
		case '5': {
			evt_t evt = { REG_SLEEP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
//		case 'd': {
//			extern int lliot(size_t pnum);
//			lliot(0);
//			break;
//		}
//		case 'i':
//			printf("\r\n");
//			cnsl_tran(cnsl_run_littlefs_st);
//			break;
		default:
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
		printf("Show continuous readings\r\n"
				"[Enter anything to quit]\r\n"
				"RPM    Volts  Amps   Int °C A11 °C\r\n"
				"------ ------ ------ ------ ------\r\n");
		break;
	case KEYSTROKE_SIG:
		printf("\r\n");
		cnsl_tran(cnsl_top_st);
		break;
	case PERIOD_1HZ_SIG: {
		data_rec_t data;
		get_data_1sec_avg(&data);
		printf("%6.3g %6.3g %6.3g %6.3g %6.3g\r\n", data.rpm, data.Bvolts,
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
		printf("\r\nUptime: %llu seconds\r\n", uptime);
		printf("Internal Temperature:\r\n\t"
				"Mean: %4.3g\r\n\t"
//				 "StdDev: %#7.3g\r\n\t"
				"Min: %6.4g\r\n\t"
				"Max: %6.4g\r\n",
				RS_Mean(&internal_temp_stats),
//				 RS_StandardDeviation(&internal_temp_stats),
				RS_Min(&internal_temp_stats),
				RS_Max(&internal_temp_stats));
		printf("B+ Voltage: \r\n\t"
				"Mean: %4.4g\r\n\t"
				"Min: %4.3g\r\n\t"
				"Max: %4.3g\r\n",
				RS_Mean(&Bplus_volt_stats),
				RS_Min(&Bplus_volt_stats),
				RS_Max(&Bplus_volt_stats));
		printf("B+ Current:\r\n\t"
				"Mean: %6.4g\r\n\t"
				"Min: %6.3g\r\n\t"
				"Max: %6.3g\r\n",
				RS_Mean(&Bplus_amp_stats),
				RS_Min(&Bplus_amp_stats),
				RS_Max(&Bplus_amp_stats));
		printf("\r\nEnter:\r\n"
				"T: Show again\r\n"
				"R: Reset statistics\r\n"
				"[anything else]: Quit\r\n");
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
			printf("\r\n");
			cnsl_tran(cnsl_top_st);
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
		printf("Configuration\r\n"
				"Settings:\r\n"
		"  V: Voltage limit: %6.3g\r\n"
		"  C: Current limit: %6.3g\r\n"
		"  P: Power limit: %6.3g\r\n"
		"  S: Save\r\n"
		"Enter V, C, or P to change,\r\n"
		"S to commit changes: ",
		cfg_get_vlim(),
		cfg_get_clim(),
		cfg_get_plim());
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
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
			printf("\r\nConfiguration saved\r\n");
			break;
		default:
			printf("\r\n");
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
		printf("Configure Voltage Limit\r\n"
				"[Enter new limit in volts,\r\n"
				"or anything else to quit]\r\n");
		printf("Existing: %6.3g\r\n", cfg_get_vlim());
		printf("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end <= sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || 'n' == c) {
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
		printf("Configure Current Limit\r\n"
				"[Enter new limit in amps,\r\n"
				"or anything else to quit]\r\n");
		printf("Existing: %6.3g\r\n", cfg_get_clim());
		printf("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end <= sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || 'n' == c) {
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
		printf("Configure Power Limit\r\n"
				"[Enter new limit in watts,\r\n"
				"or anything else to quit]\r\n");
		printf("Existing: %6.3g\r\n", cfg_get_plim());
		printf("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end <= sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || 'n' == c) {
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
		printf("Calibration\r\n"
		"  V: Voltage sense: %6.3g\r\n"
		"  C: Current sense: %6.3g\r\n"
		"Enter V or C to calibrate,\r\n"
		"or anything else to quit: ",
		data.Bvolts, data.Bamps);
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case 'v':
			cnsl_tran(cnsl_cal_v_st);
			break;
		case 'c':
			cnsl_tran(cnsl_cal_c_st);
			break;
		default:
			printf("\r\n");
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
		printf("Calibrate Voltage Sense\r\n"
				"[Enter true voltage, \r\n"
				"or anything else to quit] \r\n");
		printf("Sensed: %6.3g\r\n", data.Bvolts);
		printf("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end <= sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || 'n' == c) {
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
			cnsl_tran(cnsl_cfg_st);
		}
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
		printf("Calibrate Voltage Sense\r\n"
				"[Enter true voltage, \r\n"
				"or anything else to quit] \r\n");
		printf("Sensed: %6.3g\r\n", data.Bvolts);
		printf("New: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		cnsl.buf_end = 0;
		break;
	}
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		if ('.' == c || isdigit(c)) {
			if (cnsl.buf_end <= sizeof cnsl.buf)
				cnsl.buf[cnsl.buf_end++] = c;
		} else if (c == '\b' || c == (char)127) { // backspace
			if (cnsl.buf_end > 0)
				cnsl.buf[--cnsl.buf_end] = 0;
		} else if ('\r' == c || 'n' == c) {
			float truth;
			int rc = sscanf(cnsl.buf, "%f", &truth);
			if (1 == rc) {
				data_rec_t data;
				get_data_1sec_avg(&data);

				evt_t evt = {REG_STOP_SIG, { 0 } };
				osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
				assert(osOK == rc);

				uint16_t raw_zero = get_Bplus_amps_raw();

				evt.sig = REG_START_SIG;
				rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
				assert(osOK == rc);

				cfg_set_Bplus_amp_scale(truth / (data.Bamps_raw - raw_zero));
				cfg_set_Bplus_amp_zero(raw_zero);
				cfg_save();
			}
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
