#include <ctype.h> // tolower
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>     /* strtoul */

#include "main.h"
#include "freertos.h"
#include "analog.h"
#include "digital.h"
#include "tim.h"

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

static void cnsl_show_volt_st(evt_t const *const pEvt);
static void cnsl_show_curr_st(evt_t const *const pEvt);
static void cnsl_show_tach_st(evt_t const *const pEvt);

static void cnsl_top_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf("Voltage Regulator\r\n"
				"1 or n: Show statistics\r\n"
				"r: Reset statistics\r\n"
//				"2: Calibrate\r\n"
//				"3: Configure\r\n"
				"4 or v: Show B+ voltage\r\n"
				"5 or c: Show B+ current\r\n"
				"t: show RPM\r\n"
				"s: start regulator\r\n"
				"q: stop regulator\r\n"
				"f: enable field\r\n"
				"g: disable field\r\n"
				"l: sleep\r\n"
				"d: test SD card\r\n"
				"[Type choice]: ");
		fflush(stdout);
//		memset(cnsl.buf, 0, sizeof cnsl.buf);
//		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		switch (tolower(c)) {
		case '1':
		case 'n':
			printf("\r\nUptime: %lu seconds\r\n", xTaskGetTickCount() / 1000); // FIXME: overflows
			printf("Internal Temperature:\r\n\t"
					"Now: %4.2g\r\n\t"
//					"Samples: %10u\r\n\t"
					"Mean: %4.3g\r\n\t"
//					"StdDev: %#7.3g\r\n\t"
					"Min: %6.4g\r\n\t"
					"Max: %6.4g\r\n", internal_temp(),
//					RS_NumDataValues(&internal_temp_stats),
					RS_Mean(&internal_temp_stats),
//					RS_StandardDeviation(&internal_temp_stats),
					RS_Min(&internal_temp_stats), RS_Max(&internal_temp_stats));
//				printf("A1: Raw: %hu, Num: %10u, "
//						"Mean: %4.5g, StdDev: %#7.3g, Min: %4.3g, Max: %4.3g\r\n",
//						raw_recs[ix].A1, RS_NumDataValues(&A1_stats),
//						RS_Mean(&A1_stats), RS_StandardDeviation(&A1_stats),
//						RS_Min(&A1_stats), RS_Max(&A1_stats));
			printf("B+ Voltage: %4.3g\r\n\t"
//					"Samples: %10u\r\n\t"
							"Mean: %4.4g\r\n\t"
//					"StdDev: %#7.3g\r\n\t"
							"Min: %4.3g\r\n\t"
							"Max: %4.3g\r\n\r\n",
			// line up: "\e[1A\e[1A"
					Bplus_volt(),
//					RS_NumDataValues(&Bplus_volt_stats),
					RS_Mean(&Bplus_volt_stats),
//					RS_StandardDeviation(&Bplus_volt_stats),
					RS_Min(&Bplus_volt_stats), RS_Max(&Bplus_volt_stats));
			printf(
					"B+ Current:\r\n\t"
							"Now: %6.3g\r\n\t"
							"Samples: %10u\r\n\t"
							"Mean: %6.4g\r\n\t"
//					"StdDev: %#7.3g\r\n\t"
							"Min: %6.3g\r\n\t"
							"Max: %6.3g\r\n", Bplus_amp(),
					RS_NumDataValues(&Bplus_amp_stats),
					RS_Mean(&Bplus_amp_stats),
//					RS_StandardDeviation(&Bplus_amp_stats),
					RS_Min(&Bplus_amp_stats), RS_Max(&Bplus_amp_stats));
			fflush(stdout);
//			cnsl_dispatch(&cnsl_entry_evt);
			break;
		case 'r':
			printf("\r\n");
			ResetStats();
			printf("Statistics reset\r\n");
			break;
		case '4':
		case 'v':
			printf("\r\n");
			cnsl_tran(cnsl_show_volt_st);
			break;
		case '5':
		case 'c':
			printf("\r\n");
			cnsl_tran(cnsl_show_curr_st);
			break;
		case 's': {
			printf("\r\n");
//			enable_33_pwr(true);
			evt_t evt = { REG_START_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case 'q': {
			printf("\r\n");
//			enable_33_pwr(false);
			evt_t evt = { REG_STOP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case 'f':
			printf("\r\n");
			enable_field(true);
			printf("field enabled\r\n");
			break;
		case 'g':
			printf("\r\n");
			enable_field(false);
			printf("field disabled\r\n");
			break;
		case 'l': {
			evt_t evt = { REG_SLEEP_SIG, { 0 } };
			osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
			assert(osOK == rc);
			break;
		}
		case 'd': {
			extern int lliot(size_t pnum);
			lliot(0);
			break;
		}
		case 't':
			printf("\r\n");
			cnsl_tran(cnsl_show_tach_st);
			break;
//		case '2':
//		case '3':
		default:
			cnsl_dispatch(&cnsl_entry_evt);
		}
	}
	default:
		;
	}
}
static void cnsl_show_volt_st(evt_t const *const pEvt) {
//	float *pOldV = (float*) cnsl.buf;
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
//		*pOldV = 0.0;
		printf("Volts: [Type any key to quit]\r\n");
		break;
	case KEYSTROKE_SIG:
		printf("\r\n");
		cnsl_tran(cnsl_top_st);
		break;
	case PERIOD_10HZ_SIG: {
		float v = Bplus_volt();
//		if (fabs(*pOldV - v) >= FLT_EPSILON * fmaxf(fabs(*pOldV), fabs(v))) {
			printf("\t%4.3g\t\r", v);
			fflush(stdout);
//			*pOldV = v;
//		}
		break;
	}
	default:
		;
	}
}
static void cnsl_show_curr_st(evt_t const *const pEvt) {
	float *pOldC = (float*) cnsl.buf;
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		*pOldC = 0.0;
		printf("Current (A): [Type any key to quit]\r\n");
		break;
	case KEYSTROKE_SIG:
		printf("\r\n");
		cnsl_tran(cnsl_top_st);
		break;
	case PERIOD_10HZ_SIG: {
		float c = Bplus_amp();
		if (fabs(*pOldC - c) >= FLT_EPSILON * fmaxf(fabs(*pOldC), fabs(c))) {
			printf("\t%6.3g        \t\r", c);
			fflush(stdout);
			*pOldC = c;
		}
		break;
	}
	default:
		;
	}
}

static void cnsl_show_tach_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf("RPM: [Type any key to quit]\r\n");
		break;
	case KEYSTROKE_SIG:
		printf("\r\n");
		cnsl_tran(cnsl_top_st);
		break;
	case PERIOD_10HZ_SIG: {
		float f = frequency();
		// * sec/min * rev/fire
		float rpm = f * 60 / 2;
		printf("\t%6.1f        \t\r", rpm);
		fflush(stdout);
		break;
	}
	default:
		;
	}
}


//// calculate the mean iteratively
//++stats->num_30min;
//stats->avg_30min += (chn_val - stats->avg_30min) / stats->num_30min;
