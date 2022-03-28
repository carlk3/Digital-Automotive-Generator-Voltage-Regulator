/*
 * regulator_sm.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#include <freertos.c.h>
#include <stdio.h>
//
#include "main.h"
#include "stm32l4xx_hal_pwr_ex.h" // HAL_PWREx_EnterSTOP2Mode
//
#include "analog.h"
#include "comp.h"
#include "data.h"
#include "digital.h"
#include "config.h"
#include "global.h"
//
#include "regulator_sm.h"
//
#include "logger_sm.h"

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

static reg_t reg = {reg_run_st}; // the state machine instance {initial state}
void reg_dispatch(evt_t const *evt) {
	(reg.state)(evt);
}
static void reg_tran(reg_state_t target) {
	reg_dispatch(&reg_exit_evt); /* exit the source */
	reg.state = target;
	reg_dispatch(&reg_entry_evt); /* enter the target */
}

static void flash(unsigned n) {
	HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_RESET);
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < n; ++i) {
			HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_SET);
			/* Insert delay 100 ms */
			osDelay(200);
			HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_RESET);
			/* Insert delay 100 ms */
			osDelay(200);
		}
		osDelay(500);
	}
}

static void reg_sleep_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		log_printf("Going to sleep\r\n");
		osTimerStop(PeriodHandle);
		osTimerStop(Period1HzHandle);
		osEventFlagsClear(TaskStoppedHandle, TASK_LOG);
		evt_t evt = { LOG_STOP_SIG, { 0 } };
		osStatus_t rc = osMessageQueuePut(LoggerEvtQHandle, &evt, 0, 3000);
		if (osOK != rc) {
			printf("Dropped LOG_STOP_SIG to LoggerEvtQ\n");
		} else {
			uint32_t flags = osEventFlagsWait(TaskStoppedHandle, TASK_LOG, osFlagsWaitAny, 3000);
			configASSERT(!(0x80000000 & flags));
		}
		enable_pwr(false);
		enable_field(true); // So D+ comes up if gen starts to spin, and we wake up
		// See vPortSuppressTicksAndSleep in
		//    /VReg/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

//		LL_USART_EnableInStopMode(USART2);  // Allow to wake up

		printf("\r\n\nEntering Stop Mode\r\n\n");
		flash(3);
		HAL_SuspendTick();
		__disable_irq();
		__DSB();
		__ISB();
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFE);
		// sleeping until wakeup event...
		reg_tran(reg_run_st);
		break;
	case REG_EXIT_SIG: {
		/* Exit with interrupts enabled. */
		__enable_irq();
		SystemClock_Config();

//		/* PeriphClkInit  pointer to an RCC_PeriphCLKInitTypeDef structure that
//		  *         contains a field PeriphClockSelection which can be a combination of the following values:
//		  *            @arg @ref RCC_PERIPHCLK_ADC  ADC peripheral clock
//		  *            ...
//		  */
//		RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
//		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
//		HAL_StatusTypeDef hs = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
//		configASSERT(HAL_OK == hs);

//		__HAL_RCC_ADC_CLK_ENABLE();

		HAL_ResumeTick();
		enable_pwr(true);
//		start_adc();
		osTimerStart(Period1HzHandle, period1Hz);
		osTimerStart(PeriodHandle, regulator_period);
		evt_t evt = { LOG_START_SIG, { 0 } };
		osStatus_t rc = osMessageQueuePut(LoggerEvtQHandle, &evt, 0, 3000);
		if (osOK != rc)
			printf("Dropped LOG_START_SIG to LoggerEvtQ\n");
		log_printf("\r\nExited Stop Mode\r\n");
		break;
	}
	default:;
	}
}

static void reg_failed_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG:
		log_printf("Regulator entering failed state\r\n");
		enable_field(false);
		break;
	case REG_SLEEP_SIG:
		reg_tran(reg_sleep_st);
		break;
	default:;
	}
}

static void reg_idle_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG: {
		log_printf("Regulator idle\r\n");
		enable_field(false);
		uint32_t flags = osEventFlagsSet(TaskStoppedHandle, TASK_REG);
		configASSERT(!(0x80000000 & flags));
		break;
	}
	case REG_START_SIG:
		reg_tran(reg_run_st);
		break;
	case REG_SLEEP_SIG:
		reg_tran(reg_sleep_st);
		break;
	default:;
	}
}
static void reg_run_st(evt_t const *const pEvt) {
	switch (pEvt->sig) {
	case REG_ENTRY_SIG: {
		enable_pwr(true);
		data_rec_t data;
		get_data(&data);
		update_avgs(&data);
		log_printf("Regulator started\r\n");
		break;
	}
	case REG_STOP_SIG:
		reg_tran(reg_idle_st);
		break;
	case REG_SLEEP_SIG:
		reg_tran(reg_sleep_st);
		break;
	case PERIOD_SIG: {
		data_rec_t data, avg_data;
		get_data(&data);
		update_avgs(&data);
		get_data_1sec_avg(&avg_data);
		float clim = cfg_get_clim();
		float vlim = cfg_get_vlim();

		if (avg_data.Bamps > 1.1 * clim && data.Bvolts < 0.1 * vlim) {
			// Short?
			log_printf("Probable short!\r\n");
			reg_tran(reg_failed_st);
			break;
		}
		if (data.Bvolts > 1.1 * vlim && avg_data.Bamps < 0.1 * clim) {
			// Open?
			log_printf("Probable open!\r\n");
			reg_tran(reg_failed_st);
			break;
		}
		if (data.internal_temp > 125.0) {
			log_printf("Over temp! Internal temperature %.0f°C\n", data.internal_temp);
			reg_tran(reg_failed_st);
			break;
		}
		bool next_enable = true;
		if (data.Bvolts * data.Bamps > cfg_get_plim())
			next_enable = false;
		if (data.Bamps > clim)
			next_enable = false;
		if (data.Bvolts > vlim)
			next_enable = false;
		enable_field(next_enable);

		if (next_enable)
			++field_on_count;
		else
			++field_off_count;
		update_stats(&data);
		break;
	}
	case PERIOD_1HZ_SIG:
		update_duty_cycle();
		HAL_GPIO_TogglePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin);

		/* Keep alive if generator is spinning */

		/*  Check if COMP1 output level is high, indicating D+ high */
		if (!osTimerIsRunning(ActivityTimerHandle)
				|| (HAL_COMP_GetOutputLevel(&hcomp1)) == COMP_OUTPUT_LEVEL_HIGH) {
			osStatus_t rc = osTimerStart(ActivityTimerHandle, 60*1000); // Restart timer
			configASSERT(osOK == rc);
		}
		break;
	case REG_EXIT_SIG:
		log_printf("\nRegulator stopped\r\n");
		enable_field(false);
		break;
	default:;
	}
}

