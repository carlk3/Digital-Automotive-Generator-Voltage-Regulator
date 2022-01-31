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
// 80 MHz clock / Clock Prescaler 10 => 1 cycle = .125 µs
// ADC:
//   Channel Temperature Sensor:
//      t S_temp (1) ADC sampling time when reading the temperature 5 - - µs
//         5 / .125 is >= 40 cycles
// 80 MHz clock / Clock Prescaler 16 => 1 cycle = .2 µs
//	5 / .2 is >= 25 cycles
// 80 MHz clock / Clock Prescaler 8 => 1 cycle = .1 µs
//	5 / .1 is >= 50
// 80 MHz clock / Clock Prescaler 6 =>  1 cycle = .075 µs
// 5 / .075 is >= 67
// 80 MHz clock / Clock Prescaler 4 =>  1 cycle =  .05 µs
// 5 / .05 is >= 100
// 4 MHz clock / Clock Prescaler 1  => 1 cycle = .25 µs
// ADC:
//   Channel Temperature Sensor:
//      5 / .25 is >= 20 cycles
//
// 80×10⁶ Hz ÷ Clock Prescaler 10 ÷ 47.5 cycle sampling time ÷ 16x oversampling = 10526 samples/s
//     ÷ 8 conversions/sequence => 1316 samples/s (if they all have a 47.5 cycle sampling time).

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "comp.h"
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>
#include <stdio.h>

#include "analog.h"
#include "config.h"
#include "data.h"
//
#include "printf.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern COMP_HandleTypeDef hcomp1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void flash(unsigned n, bool forever) {
	HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_RESET);
	for (size_t j = 0; j < 3 || forever; ++j) {
		for (size_t i = 0; i < n; ++i) {
			HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_SET);
			/* Insert delay 100 ms */
			HAL_Delay(200);
			HAL_GPIO_WritePin(D2___Red_LED_GPIO_Port, D2___Red_LED_Pin, GPIO_PIN_RESET);
			/* Insert delay 100 ms */
			HAL_Delay(200);
		}
		HAL_Delay(500);
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
	uwTickPrio = TICK_INT_PRIORITY; // Avoid assert_failed in HAL_InitTick
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
  MX_SPI1_Init();
  MX_RTC_Init();
  MX_COMP1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

	/* Start analog data conversion */
	if (HAL_OK != HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED))
		Error_Handler();

	if (HAL_OK != HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&raw_recs, hadc1.Init.NbrOfConversion))
		Error_Handler();

	  /* Start COMP1 */
	if (HAL_COMP_Start(&hcomp1) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	// Start the TIMER TIM1 in the Input capture interrupt mode.
	// This is the timer for measuring frequency (for RPM)
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	cfg_load();

	RS_init(&internal_temp_stats);
	RS_init(&Bplus_volt_stats);
	RS_init(&Bplus_amp_stats);

	setbuf(stdout, NULL); // unbuffered stdout
	setvbuf (stdout, 0, _IONBF, 0);
	printf("\e[2J\e[H"  // Clear Screen
					"Voltage Regulator Console\r\n\n");

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */

__attribute__((used))
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
volatile uint32_t __attribute__((unused)) r0;
volatile uint32_t __attribute__((unused)) r1;
volatile uint32_t __attribute__((unused)) r2;
volatile uint32_t __attribute__((unused)) r3;
volatile uint32_t __attribute__((unused)) r12;
volatile uint32_t __attribute__((unused)) lr; /* Link register. */
volatile uint32_t __attribute__((unused)) pc; /* Program counter. */
volatile uint32_t __attribute__((unused)) psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; ) __BKPT(4);
}
/**
  * @brief This function handles Hard fault interrupt.
  * See https://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html
  */
__attribute__((naked))
void HardFault_Handler(void) {
#if 0
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
#else
	__asm volatile (
			" movs r0,#4       \n"
			" movs r1, lr      \n"
			" tst r0, r1       \n"
			" beq _MSP2         \n"
			" mrs r0, psp      \n"
			" b _HALT2          \n"
			"_MSP2:               \n"
			" mrs r0, msp      \n"
			"_HALT2:              \n"
			" ldr r1,[r0,#20]  \n"
			" b prvGetRegistersFromStack \n"
	);
#endif
}

__attribute__((__noreturn__))
void my_assert_func(const char *file, int line, const char *func,
                    const char *pred) {
    printf("%s: assertion \"%s\" failed: file \"%s\", line %d, function: %s\n",
        pcTaskGetName(NULL), pred, file, line, func);
    fflush(stdout);
    vTaskSuspendAll();
    taskDISABLE_INTERRUPTS();
    for( ;; ) __BKPT(3); // Stop in GUI as if at a breakpoint (if debugging,
                     // otherwise loop forever)
}

__attribute__((__noreturn__))
void __assert_func( const char *filename, int line, const char *assert_func, const char *expr ) {
	my_assert_func(filename, line, assert_func, expr);
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16) {
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
	flash(10, true);
	printf("Error!\r\n");
	__disable_irq();
	while (1) {
		__BKPT(0);
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
	Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

