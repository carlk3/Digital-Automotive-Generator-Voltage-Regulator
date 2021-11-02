#include "global.h"

float Bplus_volt, Bplus_amp, internal_temp;
float Bplus_volt_scale = 6.87f / 34060; // voltmeter / raw adc sample
RunningStat internal_temp_stats, A0_stats, A1_stats, Bplus_volt_stats, Bplus_amp_stats;
