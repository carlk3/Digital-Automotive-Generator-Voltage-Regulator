/*
 * freertos.h
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#ifndef INC_FREERTOS_C_H_
#define INC_FREERTOS_C_H_

#include "cmsis_os2.h"

extern osMessageQueueId_t LoggerEvtQHandle;
extern osMessageQueueId_t ConsoleEvtQHandle;
extern osMessageQueueId_t RegulatorEvtQHandle;
extern osTimerId_t PeriodHandle;
extern osTimerId_t Period1HzHandle;
extern osTimerId_t ActivityTimerHandle;
extern osTimerId_t CnslActivityTimerHandle;
extern osMutexId_t avg_data_mutexHandle;
extern osEventFlagsId_t TaskReadyHandle;
extern osEventFlagsId_t TaskStoppedHandle;

#endif /* INC_FREERTOS_C_H_ */
