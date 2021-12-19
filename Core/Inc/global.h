/*
 * global.h
 *
 *  Created on: Nov 1, 2021
 *      Author: carlk
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>
#include "running_stat.h"

extern const uint32_t regulator_period;
extern const uint32_t period10Hz;

extern float Bplus_volt_scale;
extern uint16_t Bplus_zero;
extern float Bplus_amp_scale;

#endif /* INC_GLOBAL_H_ */
