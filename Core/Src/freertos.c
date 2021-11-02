/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Logger */
osThreadId_t LoggerHandle;
const osThreadAttr_t Logger_attributes = {
  .name = "Logger",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Console */
osThreadId_t ConsoleHandle;
const osThreadAttr_t Console_attributes = {
  .name = "Console",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Central */
osThreadId_t CentralHandle;
const osThreadAttr_t Central_attributes = {
  .name = "Central",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CentralEvtQ */
osMessageQueueId_t CentralEvtQHandle;
const osMessageQueueAttr_t CentralEvtQ_attributes = {
  .name = "CentralEvtQ"
};
/* Definitions for ConsoleEvtQ */
osMessageQueueId_t ConsoleEvtQHandle;
const osMessageQueueAttr_t ConsoleEvtQ_attributes = {
  .name = "ConsoleEvtQ"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void LoggerTask(void *argument);
void ConsoleTask(void *argument);
void CentralTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CentralEvtQ */
  CentralEvtQHandle = osMessageQueueNew (32, sizeof(evt_t), &CentralEvtQ_attributes);

  /* creation of ConsoleEvtQ */
  ConsoleEvtQHandle = osMessageQueueNew (16, sizeof(evt_t), &ConsoleEvtQ_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Logger */
  LoggerHandle = osThreadNew(LoggerTask, NULL, &Logger_attributes);

  /* creation of Console */
  ConsoleHandle = osThreadNew(ConsoleTask, NULL, &Console_attributes);

  /* creation of Central */
  CentralHandle = osThreadNew(CentralTask, NULL, &Central_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_LoggerTask */
/**
 * @brief  Function implementing the Logger thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_LoggerTask */
void LoggerTask(void *argument)
{
  /* USER CODE BEGIN LoggerTask */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
  /* USER CODE END LoggerTask */
}

/* USER CODE BEGIN Header_ConsoleTask */
/**
 * @brief Function implementing the Console thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_ConsoleTask */
void ConsoleTask(void *argument)
{
  /* USER CODE BEGIN ConsoleTask */
	/* Infinite loop */
	for (;;) {
		//    osDelay(1);
		evt_t evt = { 0, {0} };
		osMessageQueueGet(ConsoleEvtQHandle, &evt, 0, osWaitForever);
	}
  /* USER CODE END ConsoleTask */
}

/* USER CODE BEGIN Header_CentralTask */
/**
 * @brief Function implementing the Central thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_CentralTask */
void CentralTask(void *argument)
{
  /* USER CODE BEGIN CentralTask */
	/* Infinite loop */
	for (;;) {
//    osDelay(1);
		evt_t evt = { 0, {0} };
		osMessageQueueGet(CentralEvtQHandle, &evt, 0, osWaitForever);
	}
  /* USER CODE END CentralTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
