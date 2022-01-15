/*
 * logger.h
 *
 *  Created on: Jan 14, 2022
 *      Author: carlk
 */

#ifndef INC_LOGGER_SM_H_
#define INC_LOGGER_SM_H_

#include "main.h"

void log_dispatch(evt_t const *evt);
void log_printf( const char * const pcFormat, ... ) __attribute__ ((format (__printf__, 1, 2)));

//void log_putch(char character);

#endif /* INC_LOGGER_SM_H_ */
