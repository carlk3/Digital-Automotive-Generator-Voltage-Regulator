#include "global.h"

float Vlim = 7.5;
float Ilim = 45;
float Plim = 180;

const uint32_t regulator_period =  10; // ms
const uint32_t period1Hz =  1000; // ms

float Bplus_volt_scale = 6.87f / 34060; // voltmeter / raw adc sample

//uint16_t Bplus_zero = 49880;
uint16_t Bplus_zero = 9961;

// 5.0 amp: 13845
float Bplus_amp_scale = 5.0f/(13845-9961);

// 1.0a 50130
//     -49866
//        264
//             1/264
// 2.0a 50400
//     -49866
//        534
//               1/267
// 3.0a 50670
//     -49866
//        804
//			   1/268
// 4.0a 50930
//     -49866
//       1064
//			   1/266
//float Bplus_amp_scale = 1.0f/266;
