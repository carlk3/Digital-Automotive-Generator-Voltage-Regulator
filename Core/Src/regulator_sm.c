/*
 * regulator_sm.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#include <stdio.h>
//
#include "freertos.h"
#include "main.h"
#include "stm32l4xx_hal_pwr_ex.h" // HAL_PWREx_EnterSTOP2Mode
//
#include "comp.h"
#include "data.h"
#include "digital.h"
#include "global.h"
//
#include "regulator_sm.h"
//
#include "printf.h"

typedef struct reg_t reg_t;
typedef void (*reg_state_t)(evt_t const* const);
struct reg_t {
	reg_state_t state;
};
static evt_t reg_entry_evt = { REG_ENTRY_SIG, { 0 } };
static evt_t reg_exit_evt = { REG_EXIT_SIG, { 0 } };

// fwd decls:
static void reg_sleep_st(evt_t const *const pEvt);
static void reg_idle_st(evt_t const *const pEvt);
static void reg_run_st(evt_t const *const pEvt);

static reg_t reg = {reg_idle_st}; // the state machine instance
void reg_dispatch(evt_t const *evt) {
	(reg.state)(evt);
}
static void reg_tran(reg_state_t target) {
	reg_dispatch(&reg_exit_evt); /* exit the source */
	reg.state = target;
	reg_dispatch(&reg_entry_evt); /* enter the target */
}

static void reg_sleep_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		printf("Going to sleep\r\n");
		// See vPortSuppressTicksAndSleep in
		//    /VReg/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
		printf("\r\nEntering Stop Mode\r\n");
		osTimerStop(PeriodHandle);
		enable_33_pwr(false);
		HAL_SuspendTick();
		__disable_irq();
		__DSB();
		__ISB();
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFE);
		// sleeping until wakeup event...
		reg_tran(reg_idle_st);
		break;
	case REG_EXIT_SIG: {
		SystemClock_Config();
		HAL_ResumeTick();
		/* Exit with interrupts enabled. */
		__enable_irq();
		enable_33_pwr(true);
		osTimerStart(PeriodHandle, regulator_period);
		printf("\r\nExited Stop Mode\r\n");
		break;
	}
	default:;
	}
}

static void reg_failed_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		printf("Regulator entering failed state\r\n");
		  /* Stop COMP1 (don't wake up) */
		if (HAL_COMP_Stop(&hcomp1) != HAL_OK) {
			Error_Handler();
		}
		reg_tran(reg_sleep_st);
		break;
	default:;
	}
}

static void reg_idle_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		printf("Regulator idle\r\n");
		break;
	case REG_START_SIG:
		reg_tran(reg_run_st);
		break;
	case REG_SLEEP_SIG:
		reg_tran(reg_sleep_st);
		break;
	case PERIOD_SIG:
		break;
	default:;
	}
}
static void reg_run_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		printf("Regulator started\r\n");
		break;
	case REG_STOP_SIG:
		reg_tran(reg_idle_st);
		break;
	case REG_SLEEP_SIG:
		reg_tran(reg_sleep_st);
		break;
	case PERIOD_SIG: {
		data_rec_t data;
		get_data(&data);

		if (data.Bamps > 1.1 * Ilim && data.Bvolts < 0.1 * Vlim) {
			// Short?
			reg_tran(reg_failed_st);
		}
		if (data.Bvolts > 1.1 * Vlim && data.Bamps < 0.1 * Ilim) {
			// Open?
			reg_tran(reg_failed_st);
		}
		bool next_enable = true;
		if (data.Bvolts * data.Bamps > Plim)
			next_enable = false;
		if (data.Bamps > Ilim)
			next_enable = false;
		if (data.Bvolts > Vlim)
			next_enable = false;
		enable_field(next_enable);

		if (next_enable)
			++field_on_count;
		else
			++field_off_count;
		update_avgs(&data);
		update_stats(&data);
		break;
	}
	case REG_EXIT_SIG:
		printf("Regulator stopped\r\n");
		break;
	default:;
	}

}
