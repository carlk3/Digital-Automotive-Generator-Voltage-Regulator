/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_dma.h"

#include "stm32l4xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

typedef enum {
	NO_SIG,
	CNSL_ENTRY_SIG,
	CNSL_EXIT_SIG,
	REG_ENTRY_SIG,
	REG_EXIT_SIG,
	REG_START_SIG,
	REG_STOP_SIG,
	REG_SLEEP_SIG,
	KEYSTROKE_SIG,
	PERIOD_SIG,
	PERIOD_10HZ_SIG,
	END_SIG
} sig_t;
typedef union {
	void *pointer;
	uint32_t data;
} evt_data_t;
typedef struct evt {
	sig_t sig;
	evt_data_t content;
} evt_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#ifndef count_of
#define count_of(a) (sizeof(a)/sizeof((a)[0]))
#endif
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define A2___B__Current_Sense_Pin GPIO_PIN_3
#define A2___B__Current_Sense_GPIO_Port GPIOA
#define A3___B__Voltage_Sense_Pin GPIO_PIN_4
#define A3___B__Voltage_Sense_GPIO_Port GPIOA
#define A4___D__Voltage_Sense_Pin GPIO_PIN_5
#define A4___D__Voltage_Sense_GPIO_Port GPIOA
#define A5__ADC1_IN11_Pin GPIO_PIN_6
#define A5__ADC1_IN11_GPIO_Port GPIOA
#define A6___ADC1_IN12_Pin GPIO_PIN_7
#define A6___ADC1_IN12_GPIO_Port GPIOA
#define D3___ADC1_IN15_Pin GPIO_PIN_0
#define D3___ADC1_IN15_GPIO_Port GPIOB
#define D6___ADC1_IN16_Pin GPIO_PIN_1
#define D6___ADC1_IN16_GPIO_Port GPIOB
#define D10___SPI_SS_Pin GPIO_PIN_11
#define D10___SPI_SS_GPIO_Port GPIOA
#define D2___Red_LED_Pin GPIO_PIN_12
#define D2___Red_LED_GPIO_Port GPIOA
#define SPI1_SCK_Pin GPIO_PIN_3
#define SPI1_SCK_GPIO_Port GPIOB
#define SPI1_MISO_Pin GPIO_PIN_4
#define SPI1_MISO_GPIO_Port GPIOB
#define SPI1_MOSI_Pin GPIO_PIN_5
#define SPI1_MOSI_GPIO_Port GPIOB
#define D5___Regulator_Switch_Control_Pin GPIO_PIN_6
#define D5___Regulator_Switch_Control_GPIO_Port GPIOB
#define D4___3_3v_On_Off_Pin GPIO_PIN_7
#define D4___3_3v_On_Off_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
