#include "global.h"

float Bplus_volt_scale = 6.87f / 34060; // voltmeter / raw adc sample

uint16_t Bplus_zero = 49870;

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
float Bplus_amp_scale = 1.0f/266;
