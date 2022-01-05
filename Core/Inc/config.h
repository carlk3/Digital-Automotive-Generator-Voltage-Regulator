/*
 * config.h
 *
 *  Created on: Jan 2, 2022
 *      Author: carlk
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include <stdbool.h>

typedef struct {
    uint32_t signature;
	float vlim;
	float clim;
	float plim;
	float Bplus_volt_scale;
	float Bplus_amp_scale;
	uint32_t Bplus_zero; // padded to 32 bits
    uint32_t checksum;  // last, not included in checksum
} config_t;

float cfg_get_vlim();
void cfg_set_vlim(float f);

float cfg_get_clim();
void cfg_set_clim(float f);

float cfg_get_plim();
void cfg_set_plim(float f);

float cfg_get_Bplus_volt_scale();
void cfg_set_Bplus_volt_scale(float f);

float cfg_get_Bplus_amp_scale();
void cfg_set_Bplus_amp_scale(float f);

uint16_t cfg_get_Bplus_amp_zero();
void cfg_set_Bplus_amp_zero(uint16_t u);

void cfg_save();
bool cfg_load();

#endif /* INC_CONFIG_H_ */
