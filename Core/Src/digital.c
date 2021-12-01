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
		HAL_GPIO_WritePin(D4___3_3v_On_Off_GPIO_Port, D4___3_3v_On_Off_Pin,
				GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(D4___3_3v_On_Off_GPIO_Port, D4___3_3v_On_Off_Pin,
				GPIO_PIN_RESET);
}
void enable_field(bool enable) {
	if (enable)
		HAL_GPIO_WritePin(D5___Regulator_Switch_Control_GPIO_Port,
				D5___Regulator_Switch_Control_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(D5___Regulator_Switch_Control_GPIO_Port,
				D5___Regulator_Switch_Control_Pin, GPIO_PIN_RESET);
}
