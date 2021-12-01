/*
 * digital.h
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#ifndef INC_DIGITAL_H_
#define INC_DIGITAL_H_

#include <stdbool.h>

void enable_33_pwr(bool enable);
void enable_field(bool enable);

//HAL_GPIO_WritePin(GPIOA, D10___SPI_SS_Pin|D2___Red_LED_Pin, GPIO_PIN_RESET);
//HAL_GPIO_WritePin(GPIOB, D5___Regulator_Switch_Control_Pin|D4___3_3v_On_Off_Pin, GPIO_PIN_RESET);


#endif /* INC_DIGITAL_H_ */
