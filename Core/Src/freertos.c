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
#include "console_sm.h"
#include "regulator_sm.h"
#include "analog.h"
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
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Regulator */
osThreadId_t RegulatorHandle;
const osThreadAttr_t Regulator_attributes = {
  .name = "Regulator",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for RegulatorEvtQ */
osMessageQueueId_t RegulatorEvtQHandle;
const osMessageQueueAttr_t RegulatorEvtQ_attributes = {
  .name = "RegulatorEvtQ"
};
/* Definitions for ConsoleEvtQ */
osMessageQueueId_t ConsoleEvtQHandle;
const osMessageQueueAttr_t ConsoleEvtQ_attributes = {
  .name = "ConsoleEvtQ"
};
/* Definitions for Period */
osTimerId_t PeriodHandle;
const osTimerAttr_t Period_attributes = {
  .name = "Period"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void LoggerTask(void *argument);
void ConsoleTask(void *argument);
void RegulatorTask(void *argument);
void PeriodCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {

}

__weak unsigned long getRunTimeCounterValue(void) {
	return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 3 */
void vApplicationTickHook(void) {
	/* This function will be called by each tick interrupt if
	 configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
	 added here, but the tick hook is called from an interrupt context, so
	 code must not attempt to block, and only the interrupt safe FreeRTOS API
	 functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void) {
	/* vApplicationMallocFailedHook() will only be called if
	 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
	 function that will get called if a call to pvPortMalloc() fails.
	 pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 timer or semaphore is created. It is also called by various parts of the
	 demo application. If heap_1.c or heap_2.c are used, then the size of the
	 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 to query the size of free heap space that remains (although it does not
	 provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/* USER CODE BEGIN PREPOSTSLEEP */


__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
	/* place for user code */

	/* configPRE_SLEEP_PROCESSING() can
	 set its parameter to 0 to indicate that its implementation contains
	 its own wait for interrupt or wait for event instruction, and so wfi
	 should not be executed again. */
//	ulExpectedIdleTime = 0;
//
//	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFE);
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
	SystemClock_Config();
}
/* USER CODE END PREPOSTSLEEP */

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

  /* Create the timer(s) */
  /* creation of Period */
  PeriodHandle = osTimerNew(PeriodCallback, osTimerPeriodic, NULL, &Period_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */

//  osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks)

  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of RegulatorEvtQ */
  RegulatorEvtQHandle = osMessageQueueNew (32, sizeof(evt_t), &RegulatorEvtQ_attributes);

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

  /* creation of Regulator */
  RegulatorHandle = osThreadNew(RegulatorTask, NULL, &Regulator_attributes);

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
//	printf("%s\r\n", __FUNCTION__);
	evt_t evt = { CNSL_ENTRY_SIG, { 0 } };
	cnsl_dispatch(&evt);
	/* Infinite loop */
	for (;;) {
		// osStatus_t 	osMessageQueueGet (osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
		osStatus_t rc = osMessageQueueGet(ConsoleEvtQHandle, &evt, 0, osWaitForever);
		assert(osOK == rc);
		cnsl_dispatch(&evt);
	}
  /* USER CODE END ConsoleTask */
}

/* USER CODE BEGIN Header_RegulatorTask */
/**
* @brief Function implementing the Regulator thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RegulatorTask */
void RegulatorTask(void *argument)
{
  /* USER CODE BEGIN RegulatorTask */
	evt_t evt = { REG_ENTRY_SIG, { 0 } };
	reg_dispatch(&evt);
	/* Infinite loop */
	for (;;) {
		osStatus_t rc = osMessageQueueGet(RegulatorEvtQHandle, &evt, 0, osWaitForever);
		assert(osOK == rc);
		reg_dispatch(&evt);
	}
  /* USER CODE END RegulatorTask */
}

/* PeriodCallback function */
void PeriodCallback(void *argument)
{
  /* USER CODE BEGIN PeriodCallback */
	evt_t evt = { PERIOD_SIG, { 0 } };
	osStatus_t rc = osMessageQueuePut(RegulatorEvtQHandle, &evt, 0, 0);
	assert(osOK == rc);
	rc = osMessageQueuePut(ConsoleEvtQHandle, &evt, 0, 0);
	assert(osOK == rc);

	UpdateStats();
  /* USER CODE END PeriodCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
