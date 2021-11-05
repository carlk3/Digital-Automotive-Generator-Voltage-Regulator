/*
 * digital.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */
#include "main.h"

#include "digital.h"

void enable_33_pwr(bool enable) {
	if (enable)
		HAL_GPIO_WritePin(GPIOC, D7___3_3v_switch_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOC, D7___3_3v_switch_Pin, GPIO_PIN_RESET);
}
