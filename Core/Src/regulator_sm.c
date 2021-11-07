/*
 * regulator_sm.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#include <stdio.h>

#include "main.h"

#include "digital.h"

#include "regulator_sm.h"

typedef struct reg_t reg_t;
typedef void (*reg_state_t)(evt_t const* const);
struct reg_t {
	reg_state_t state;
};
static evt_t reg_entry_evt = { REG_ENTRY_SIG, { 0 } };
static evt_t reg_exit_evt = { REG_EXIT_SIG, { 0 } };

static void reg_idle_st(evt_t const *const pEvt);  // fwd decl
static reg_t reg = {reg_idle_st}; // the state machine instance
void reg_dispatch(evt_t const *evt) {
	(reg.state)(evt);
}
static void reg_tran(reg_state_t target) {
	reg_dispatch(&reg_exit_evt); /* exit the source */
	reg.state = target;
	reg_dispatch(&reg_entry_evt); /* enter the target */
}

static void reg_run_st(evt_t const *const pEvt);

static void reg_idle_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_START_SIG:
		reg_tran(reg_run_st);
		break;
	default:;
	}
}
static void reg_run_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		enable_33_pwr(true);
		printf("Regulator started\r\n");
		break;
	case REG_STOP_SIG:
		reg_tran(reg_idle_st);
		break;
	case PERIOD_SIG:
		break;
	case REG_EXIT_SIG:
		enable_33_pwr(false);
		printf("Regulator stopped\r\n");
		break;
	default:;
	}

}
