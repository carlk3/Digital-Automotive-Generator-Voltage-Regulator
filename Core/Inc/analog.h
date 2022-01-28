/*
 * analog.h
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#ifndef INC_ANALOG_H_
#define INC_ANALOG_H_

#include <stdint.h>

typedef volatile struct {
	uint16_t internal_temp;
	uint16_t PA3_A2_ADC1_IN8_B_CUR;  // B+ Current Sense
	uint16_t PA4_A3_ADC1_IN9_B_VLT;  // B+ Voltage Sense
	uint16_t PA6_A5_ADC1_IN11;
	uint16_t PA7_A6_ADC1_IN12;
	uint16_t PB0_D3_ADC1_IN15;
	uint16_t PB1_D6_ADC1_IN16;
}  __attribute__((packed, aligned (sizeof(uint32_t)))) adc_raw_rec_t;

extern adc_raw_rec_t raw_recs;

float get_internal_temp();
float get_Bplus_volts();
uint16_t get_Bplus_volts_raw();
float get_Bplus_amps();
uint16_t get_Bplus_amps_raw();
float get_ADC11_temp();
float get_ADC12_temp();
float get_ADC15_temp();
float get_ADC16_temp();

#endif /* INC_ANALOG_H_ */
