/*
 * freertos.h
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#ifndef INC_FREERTOS_H_
#define INC_FREERTOS_H_

#include "cmsis_os.h"

extern osMessageQueueId_t LoggerEvtQHandle;
extern osMessageQueueId_t ConsoleEvtQHandle;
extern osMessageQueueId_t RegulatorEvtQHandle;
extern osTimerId_t PeriodHandle;
extern osMutexId_t avg_data_mutexHandle;

#endif /* INC_FREERTOS_H_ */
