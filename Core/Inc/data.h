/*
 * data.h
 *
 *  Created on: Dec 25, 2021
 *      Author: carlk
 */

#ifndef INC_DATA_H_
#define INC_DATA_H_

#include "running_stat.h"

extern unsigned long long uptime; // Seconds since reset
extern unsigned field_on_count, field_off_count; // for duty cycle

typedef struct {
	float rpm;
	float Bvolts_raw;
	float Bvolts;
	float Bamps_raw;
	float Bamps;
	float duty_cycle;
	float internal_temp;
	float ADC11_degC;
	float ADC12_degC;
	float ADC15_degC;
	float ADC16_degC;
} data_rec_t;

void get_data(data_rec_t *p);
void get_data_1sec_avg(data_rec_t *p);
void update_avgs(data_rec_t *p);
void update_duty_cycle();

extern RunningStat internal_temp_stats, Bplus_volt_stats, Bplus_amp_stats;

void update_stats(data_rec_t *p);
void reset_stats();

#endif /* INC_DATA_H_ */
