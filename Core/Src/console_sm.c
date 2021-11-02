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
static void cnsl_dispatch(evt_t const *evt) {
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
		printf("Voltage Regulator\n"
				"1: Show statistics"
				"2: Calibrate"
				"3: Configure"
				"[Enter choice (1 - 3)]: ");
		memset(cnsl.buf, 0, sizeof cnsl.buf);
		break;
	case CNSL_KEYSTROKE_SIG: {
		char c = pEvt->content.data;
		printf("%c", c);  // echo
		switch (c) {
		case '\r':  // Enter
		{
			unsigned u = strtoul(cnsl.buf, 0, 0);
			switch (u) {
			case 1:
				printf("Uptime: %lu seconds\n", xTaskGetTickCount() / 1000); // FIXME: overflows
				printf(
						"Temp: Current: %4.2g, Num: %10u, "
								"Mean: %4.3g, StdDev: %#7.3g, Min: %6.4g, Max: %6.4g\r\n",
						internal_temp, RS_NumDataValues(&internal_temp_stats),
						RS_Mean(&internal_temp_stats),
						RS_StandardDeviation(&internal_temp_stats),
						RS_Min(&internal_temp_stats),
						RS_Max(&internal_temp_stats));
//				printf("A1: Raw: %hu, Num: %10u, "
//						"Mean: %4.5g, StdDev: %#7.3g, Min: %4.3g, Max: %4.3g\r\n",
//						raw_recs[ix].A1, RS_NumDataValues(&A1_stats),
//						RS_Mean(&A1_stats), RS_StandardDeviation(&A1_stats),
//						RS_Min(&A1_stats), RS_Max(&A1_stats));
				printf(
						"BplusV: %4.3g, Num: %10u, "
								"Mean: %4.4g, StdDev: %#7.3g, Min: %4.3g, Max: %4.3g\r\e[1A\e[1A",
						Bplus_volt, RS_NumDataValues(&Bplus_volt_stats),
						RS_Mean(&Bplus_volt_stats),
						RS_StandardDeviation(&Bplus_volt_stats),
						RS_Min(&Bplus_volt_stats), RS_Max(&Bplus_volt_stats));

				break;
			case 2:
				break;
			case 3:
				break;
			default:
				cnsl_dispatch(&cnsl_entry_evt);
			}
			break;
		}
		case '\b':
		case 0x7F: // ASCII DEL
			/* Backspace was pressed.  Erase the last character
			 in the string - if any. */
			if (cnsl.buf_end > 0) {
				cnsl.buf_end--;
				cnsl.buf[cnsl.buf_end] = '\0';
			}
			break;
		case '\e':
			cnsl_dispatch(&cnsl_entry_evt);
			break;
		default:
			if (cnsl.buf_end < sizeof cnsl.buf - 1)
				cnsl.buf[cnsl.buf_end++] = pEvt->content.data;
		}
		break;
	}

	default:
		assert(false);
	}
}

//// calculate the mean iteratively
//++stats->num_30min;
//stats->avg_30min += (chn_val - stats->avg_30min) / stats->num_30min;
