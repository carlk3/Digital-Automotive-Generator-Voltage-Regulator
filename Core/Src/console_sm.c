#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>     /* strtoul */

#include "main.h"
#include "global.h"
#include "task.h"

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

static void cnsl_top_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf("Voltage Regulator\r\n"
				"1: Show statistics\r\n"
				"2: Calibrate\r\n"
				"3: Configure\r\n"
				"4: Show B+ voltage\r\n"
				"[Type choice (1 - 4)]: ");
		fflush(stdout);
//		memset(cnsl.buf, 0, sizeof cnsl.buf);
//		cnsl.buf_end = 0;
		break;
	case KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		putchar(c);  // echo
		fflush(stdout);
		switch (c) {
		case '1':
			printf("\r\nUptime: %lu seconds\r\n", xTaskGetTickCount() / 1000); // FIXME: overflows
			printf("Internal Temperature:\r\n\t"
					"Now: %4.2g\r\n\t"
					"Readings: %10u\r\n\t"
					"Mean: %4.3g\r\n\t"
					"StdDev: %#7.3g\r\n\t"
					"Min: %6.4g\r\n\t"
					"Max: %6.4g\r\n",
					internal_temp, RS_NumDataValues(&internal_temp_stats),
					RS_Mean(&internal_temp_stats),
					RS_StandardDeviation(&internal_temp_stats),
					RS_Min(&internal_temp_stats), RS_Max(&internal_temp_stats));
//				printf("A1: Raw: %hu, Num: %10u, "
//						"Mean: %4.5g, StdDev: %#7.3g, Min: %4.3g, Max: %4.3g\r\n",
//						raw_recs[ix].A1, RS_NumDataValues(&A1_stats),
//						RS_Mean(&A1_stats), RS_StandardDeviation(&A1_stats),
//						RS_Min(&A1_stats), RS_Max(&A1_stats));
			printf("B+ Voltage: %4.3g\r\n\t"
					"Readings: %10u\r\n\t"
					"Mean: %4.4g\r\n\t"
					"StdDev: %#7.3g\r\n\t"
					"Min: %4.3g\r\n\t"
					"Max: %4.3g\r\n\r\n",
					// line up: "\e[1A\e[1A"
					Bplus_volt, RS_NumDataValues(&Bplus_volt_stats),
					RS_Mean(&Bplus_volt_stats),
					RS_StandardDeviation(&Bplus_volt_stats),
					RS_Min(&Bplus_volt_stats), RS_Max(&Bplus_volt_stats));
			fflush(stdout);
			cnsl_dispatch(&cnsl_entry_evt);
			break;
		case '4':
			printf("\r\n");
			cnsl_tran(cnsl_show_volt_st);
			break;
		case '2':
		case '3':
		default:
			cnsl_dispatch(&cnsl_entry_evt);
		}
	}
	default:
		;
	}
}
static void cnsl_show_volt_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case CNSL_ENTRY_SIG:
		printf("Volts: %4.3g\r", Bplus_volt);
		fflush(stdout);
		break;
	case KEYSTROKE_SIG:
		printf("\r\n");
		cnsl_tran(cnsl_top_st);
		break;
	case ADC_CMPLT_SIG:
		cnsl_dispatch(&cnsl_entry_evt); /* enter the target */
		break;
	default:
		;
	}
}

//// calculate the mean iteratively
//++stats->num_30min;
//stats->avg_30min += (chn_val - stats->avg_30min) / stats->num_30min;
