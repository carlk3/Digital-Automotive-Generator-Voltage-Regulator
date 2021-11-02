/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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

// Notes:
// ADC:
//   80MHz clock / Clock Prescaler 10 => 1 cycle = .125µs
//   Channel Temperature Sensor:
//      t S_temp (1) ADC sampling time when reading the temperature 5 - - µs
//         5 / .125 is >= 40 cycles

//   htim6.Init.Prescaler = 8000-1; // 80 MHz -> 10 kHz -> .1 ms
//   htim6.Init.Period = 100-1; // 10 ms -> 100 Hz

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <running_stat.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_BUF_LEN 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static float const Bplus_scale = 6.87f / 34060; // voltmeter / raw adc sample
static size_t const ADC_RAW_REC_LEN = sizeof(adc_raw_rec_t) / sizeof(uint16_t);
adc_raw_rec_t raw_recs[ADC_BUF_LEN];
static RunningStat internal_temp_stats, A0_stats, A1_stats, Bplus_stats;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
//	for (size_t ix = 0; ix < ADC_BUF_LEN/2; ++ix) {
//
//		RS_Push(&A1_stats, raw_recs[ix].A1);
//		printf("Current: %hu\t\tNum: %7u\t\tMean: %g\t\tStdDev: %g\r",
//				raw_recs[ix].A1, RS_NumDataValues(&A1_stats), RS_Mean(&A1_stats),
//				RS_StandardDeviation(&A1_stats));
//	}
//}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {

	for (size_t ix = 0; ix < ADC_BUF_LEN; ++ix) {

		evt_t evt = { ADC_CMPLT_SIG, {raw_recs} };
		osMessageQueuePut(CentralEvtQHandle, &evt, 0, 0);

		float inttemp = __LL_ADC_CALC_TEMPERATURE(3300,
				raw_recs[ix].internal_temp >> 4, LL_ADC_RESOLUTION_12B);
		RS_Push(&internal_temp_stats, inttemp);
		printf("Temp: Current: %4.2g, Num: %10u, "
				"Mean: %4.3g, StdDev: %#7.3g, Min: %6.4g, Max: %6.4g\r\n",
				inttemp, RS_NumDataValues(&internal_temp_stats),
				RS_Mean(&internal_temp_stats),
				RS_StandardDeviation(&internal_temp_stats),
				RS_Min(&internal_temp_stats), RS_Max(&internal_temp_stats));

		RS_Push(&A1_stats, raw_recs[ix].A1);
		printf("A1: Raw: %hu, Num: %10u, "
				"Mean: %4.5g, StdDev: %#7.3g, Min: %4.3g, Max: %4.3g\r\n",
				raw_recs[ix].A1, RS_NumDataValues(&A1_stats),
				RS_Mean(&A1_stats), RS_StandardDeviation(&A1_stats),
				RS_Min(&A1_stats), RS_Max(&A1_stats));

		float BplusV = (float) raw_recs[ix].A1 * Bplus_scale;
		RS_Push(&Bplus_stats, BplusV);
		printf(
				"BplusV: %4.3g, Num: %10u, "
						"Mean: %4.4g, StdDev: %#7.3g, Min: %4.3g, Max: %4.3g\r\e[1A\e[1A",
				BplusV, RS_NumDataValues(&Bplus_stats), RS_Mean(&Bplus_stats),
				RS_StandardDeviation(&Bplus_stats), RS_Min(&Bplus_stats),
				RS_Max(&Bplus_stats));

		fflush(stdout);

	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
//	setbuf(stdout, NULL); // unbuffered stdout
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

	RS_init(&internal_temp_stats);
	RS_init(&A0_stats);
	RS_init(&A1_stats);
	RS_init(&Bplus_stats);

	/* Start analog data conversion */
	if (HAL_OK != HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED))
		Error_Handler();
	printf("\e[2J\e[H");  // Clear Screen
	printf("\r\nStart\r\n");
	if (HAL_OK != HAL_ADC_Start_DMA(&hadc1, (uint32_t*) raw_recs,
	ADC_BUF_LEN * ADC_RAW_REC_LEN))
		Error_Handler();
	if (HAL_OK != HAL_TIM_Base_Start(&htim6))
		Error_Handler();

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	printf("Error!\r\n");
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	printf("Wrong parameters value: file %s on line %lu\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
