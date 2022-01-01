/*
 * data.c
 *
 *  Created on: Dec 25, 2021
 *      Author: carlk
 */
#include "freertos.h"
//
#include <string.h> // memcpy
//
#include "analog.h"
#include "global.h"
#include "tim.h"
//
#include "data.h"

unsigned long long uptime;

unsigned field_on_count, field_off_count;

void get_data(data_rec_t *p) {
	// * sec/min * rev/fire
	p->rpm = frequency() * 60 / 2;
	p->Bvolts = Bplus_volt();
	p->Bamps = Bplus_amp();
	if (field_on_count + field_off_count)
		p->duty_cycle = field_on_count / (field_on_count + field_off_count);
	else
		p->duty_cycle = 0;
	p->internal_temp = internal_temp();
	p->ADC11_degC = ADC11_temp();
	p->ADC12_degC = ADC12_temp();
	p->ADC15_degC = ADC15_temp();
	p->ADC16_degC = ADC16_temp();
}

static data_rec_t avg1sec;
static unsigned N;

void update_avgs(data_rec_t *p) {
	osStatus_t stat = osMutexAcquire(avg_data_mutexHandle, osWaitForever);
	configASSERT(osOK == stat);
	if (!N) {
		memcpy(&avg1sec, p, sizeof avg1sec);
		++N;
	} else {
		// calculate the mean iteratively
		// #define configTICK_RATE_HZ ((TickType_t)1000)
		// regulator_period =  10; // ms
		if (N < configTICK_RATE_HZ / regulator_period)
			++N;
		avg1sec.rpm += (p->rpm - avg1sec.rpm) / N;
		avg1sec.Bvolts += (p->Bvolts - avg1sec.Bvolts) / N;
		avg1sec.Bamps += (p->Bamps - avg1sec.Bamps) / N;
		avg1sec.duty_cycle += (p->duty_cycle - avg1sec.duty_cycle) / N;
		avg1sec.internal_temp += (p->internal_temp - avg1sec.internal_temp) / N;
		avg1sec.ADC11_degC += (p->ADC11_degC - avg1sec.ADC11_degC) / N;
		avg1sec.ADC12_degC += (p->ADC12_degC - avg1sec.ADC12_degC) / N;
		avg1sec.ADC15_degC += (p->ADC15_degC - avg1sec.ADC15_degC) / N;
		avg1sec.ADC16_degC += (p->ADC16_degC - avg1sec.ADC16_degC) / N;
	}
	stat = osMutexRelease(avg_data_mutexHandle);
	configASSERT(osOK == stat);
}

void get_data_1sec_avg(data_rec_t *p) {
	osStatus_t stat = osMutexAcquire(avg_data_mutexHandle, osWaitForever);
	configASSERT(osOK == stat);
	memcpy(p, &avg1sec, sizeof avg1sec);
	stat = osMutexRelease(avg_data_mutexHandle);
	configASSERT(osOK == stat);
}

RunningStat internal_temp_stats, Bplus_volt_stats, Bplus_amp_stats;

void update_stats(data_rec_t *p) {
	RS_Push(&internal_temp_stats, p->internal_temp);
	RS_Push(&Bplus_volt_stats, p->Bvolts);
	RS_Push(&Bplus_amp_stats, p->Bamps);
}
void reset_stats() {
	RS_Clear(&internal_temp_stats);
	RS_Clear(&Bplus_volt_stats);
	RS_Clear(&Bplus_amp_stats);
	uptime = 0;
}

