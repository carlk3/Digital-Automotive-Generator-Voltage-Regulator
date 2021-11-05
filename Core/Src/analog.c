/*
 * analog.c
 *
 *  Created on: Nov 4, 2021
 *      Author: carlk
 */

#include "main.h"
#include "freertos.h"
#include "running_stat.h"
#include "global.h"

#include "analog.h"

adc_raw_rec_t raw_recs[ADC_BUF_LEN];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {

	for (size_t ix = 0; ix < ADC_BUF_LEN; ++ix) {

		evt_t evt = { ADC_CMPLT_SIG, .content.pointer = &raw_recs[ix] };
//		osStatus_t rc = osMessageQueuePut(CentralEvtQHandle, &evt, 0, 0);
//		assert(osOK == rc);

		internal_temp = __LL_ADC_CALC_TEMPERATURE(3300,
				raw_recs[ix].internal_temp >> 4, LL_ADC_RESOLUTION_12B);
		RS_Push(&internal_temp_stats, internal_temp);

		RS_Push(&A1_stats, raw_recs[ix].PA3_A2);

		Bplus_volt = (float) raw_recs[ix].PA3_A2 * Bplus_volt_scale;
		RS_Push(&Bplus_volt_stats, Bplus_volt);

		osStatus_t rc = osMessageQueuePut(ConsoleEvtQHandle, &evt, 0, 0);
		assert(osOK == rc);

	}
}

