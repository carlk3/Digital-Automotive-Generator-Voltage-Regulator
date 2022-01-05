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
#include "config.h"

#include "analog.h"

adc_raw_rec_t raw_recs;

float get_internal_temp() {
	return __LL_ADC_CALC_TEMPERATURE(3300, raw_recs.internal_temp >> 4,
			LL_ADC_RESOLUTION_12B);
}
float get_Bplus_volts() {
	return raw_recs.PA4_A3_ADC1_IN9_B_VLT * cfg_get_Bplus_volt_scale();
}
uint16_t get_Bplus_volts_raw() {
	return raw_recs.PA4_A3_ADC1_IN9_B_VLT;
}
float get_Bplus_amps() {
	return (raw_recs.PA3_A2_ADC1_IN8_B_CUR - cfg_get_Bplus_amp_zero()) * cfg_get_Bplus_amp_scale();
//	return raw_recs.PA3_A2_ADC1_IN8_B_CUR;
}
uint16_t get_Bplus_amps_raw() {
	return raw_recs.PA3_A2_ADC1_IN8_B_CUR;
}

// Temp in °C = [(Vout in mV) - 500] / 10
// Vref = 3.3
// 16 bits with oversampling
// 3.3/65535 volts/n
// Scale Factor, TMP36 −40°C ≤ TA ≤ +125°C 10 mV/°C
// TMP36 Output Voltage TA = 25°C 750 mV
float get_ADC11_temp() {
	float Vout = raw_recs.PA6_A5_ADC1_IN11 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
float get_ADC12_temp() {
	float Vout = raw_recs.PA7_A6_ADC1_IN12 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
float get_ADC15_temp() {
	float Vout = raw_recs.PB0_D3_ADC1_IN15 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
float get_ADC16_temp() {
	float Vout = raw_recs.PB1_D6_ADC1_IN16 * 3.3f/65535;
	return (Vout * 1000 - 500) / 10;
}
