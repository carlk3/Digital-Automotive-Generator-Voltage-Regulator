/*
 * global.h
 *
 *  Created on: Nov 1, 2021
 *      Author: carlk
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "running_stat.h"

extern float Bplus_volt, Bplus_amp, internal_temp;
extern float Bplus_volt_scale;
extern RunningStat internal_temp_stats, A0_stats, A1_stats, Bplus_volt_stats, Bplus_amp_stats;

#endif /* INC_GLOBAL_H_ */
