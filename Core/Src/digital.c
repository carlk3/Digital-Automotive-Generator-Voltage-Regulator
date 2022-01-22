/*
 * digital.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */
#include "main.h"

#include "digital.h"

void enable_pwr(bool enable) {
	if (enable)
		HAL_GPIO_WritePin(D4___Pwr_On_Off_GPIO_Port, D4___Pwr_On_Off_Pin,
				GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(D4___Pwr_On_Off_GPIO_Port, D4___Pwr_On_Off_Pin,
				GPIO_PIN_RESET);
}
void enable_field(bool enable) {
	if (enable) {
		HAL_GPIO_WritePin(D5___Regulator_Switch_Control_GPIO_Port,
				D5___Regulator_Switch_Control_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port,
//				D2___Red_LED_Pin, GPIO_PIN_SET);
	} else {
//		HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port,
//				D2___Red_LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(D5___Regulator_Switch_Control_GPIO_Port,
				D5___Regulator_Switch_Control_Pin, GPIO_PIN_RESET);
	}
}
