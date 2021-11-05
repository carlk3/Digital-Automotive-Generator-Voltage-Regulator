/*
 * analog.h
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#ifndef INC_ANALOG_H_
#define INC_ANALOG_H_

#define ADC_BUF_LEN 1
typedef struct {
	uint16_t internal_temp;
	uint16_t PA3_A2;
	uint16_t PA4_A3;
	uint16_t PA5_A4;
	uint16_t PA6_A5;
	uint16_t PA7_A6;
	uint16_t PB0_D3;
	uint16_t PB1_D6;
} /* __attribute__((packed)) */ adc_raw_rec_t;
#define ADC_RAW_REC_LEN (sizeof(adc_raw_rec_t) / sizeof(uint16_t))

extern adc_raw_rec_t raw_recs[ADC_BUF_LEN];

#endif /* INC_ANALOG_H_ */
