/*
 * freertos.h
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#ifndef INC_FREERTOS_H_
#define INC_FREERTOS_H_

#include "cmsis_os.h"

extern osMessageQueueId_t RegulatorEvtQHandle;
extern osMessageQueueId_t ConsoleEvtQHandle;

#endif /* INC_FREERTOS_H_ */
