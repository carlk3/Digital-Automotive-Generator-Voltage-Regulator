/*
 * analog.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */
//#include "freertos.h"
//
#include "main.h"
//
#include "global.h"

#include "analog.h"

adc_raw_rec_t raw_recs;

float internal_temp() {
	return __LL_ADC_CALC_TEMPERATURE(3300, raw_recs.internal_temp >> 4,
			LL_ADC_RESOLUTION_12B);
}
float Bplus_volt() {
	return raw_recs.PA4_A3_ADC1_IN9_B_VLT * Bplus_volt_scale;
}
float Bplus_amp() {
	return (raw_recs.PA3_A2_ADC1_IN8_B_CUR - Bplus_zero) * Bplus_amp_scale;
//	return raw_recs.PA3_A2_ADC1_IN8_B_CUR;
}

// Temp in °C = [(Vout in mV) - 500] / 10
// Vref = 3.3
// 16 bits with oversampling
// 3.3/65535 volts/n
// Scale Factor, TMP36 −40°C ≤ TA ≤ +125°C 10 mV/°C
// TMP36 Output Voltage TA = 25°C 750 mV
float ADC11_temp() {
	float Vout = raw_recs.PA6_A5_ADC1_IN11 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
float ADC12_temp() {
	float Vout = raw_recs.PA7_A6_ADC1_IN12 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
float ADC15_temp() {
	float Vout = raw_recs.PB0_D3_ADC1_IN15 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
float ADC16_temp() {
	float Vout = raw_recs.PB1_D6_ADC1_IN16 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
