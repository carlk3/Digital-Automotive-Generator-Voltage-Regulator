/*
 * analog.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#include "main.h"
#include "freertos.h"
#include "global.h"

#include "analog.h"

adc_raw_rec_t raw_recs;
RunningStat internal_temp_stats, Bplus_volt_stats, Bplus_amp_stats;

float internal_temp() {
	return __LL_ADC_CALC_TEMPERATURE(3300,
			raw_recs.internal_temp >> 4, LL_ADC_RESOLUTION_12B);
}
float Bplus_volt() {
	return raw_recs.PA4_A3_ADC1_IN9_B_VLT * Bplus_volt_scale;
}
float Bplus_amp() {
	return (raw_recs.PA3_A2_ADC1_IN8_B_CUR - Bplus_zero) * Bplus_amp_scale;
//	return raw_recs.PA3_A2_ADC1_IN8_B_CUR;
}

void UpdateStats() {
	RS_Push(&internal_temp_stats, internal_temp());
	RS_Push(&Bplus_volt_stats, Bplus_volt());
	RS_Push(&Bplus_amp_stats, Bplus_amp());
}
void ResetStats() {
	RS_Clear(&internal_temp_stats);
	RS_Clear(&Bplus_volt_stats);
	RS_Clear(&Bplus_amp_stats);
}
