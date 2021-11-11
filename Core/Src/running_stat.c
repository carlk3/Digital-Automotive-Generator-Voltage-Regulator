/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
 */
// Derived from original at
//    https://www.johndcook.com/blog/standard_deviation/
#include <float.h>
#include <math.h>

#include <running_stat.h>

// public:
void RS_init(RunningStat *const pRS) {
	pRS->m_n = 0;
	pRS->m_min = FLT_MAX;
	pRS->m_max = FLT_MIN;
}

void RS_Clear(RunningStat *const pRS) {
	pRS->m_n = 0;
	pRS->m_min = FLT_MAX;
	pRS->m_max = FLT_MIN;
}

void RS_Push(RunningStat *const pRS, const float x) {
	pRS->m_n++;

	// See Knuth TAOCP vol 2, 3rd edition, page 232
	if (pRS->m_n == 1) {
		pRS->m_oldM = pRS->m_newM = x;
		pRS->m_oldS = 0.0;
	} else {
		pRS->m_newM = pRS->m_oldM + (x - pRS->m_oldM) / pRS->m_n;
		pRS->m_newS = pRS->m_oldS + (x - pRS->m_oldM) * (x - pRS->m_newM);

		// set up for next iteration
		pRS->m_oldM = pRS->m_newM;
		pRS->m_oldS = pRS->m_newS;
	}

	if (x < pRS->m_min)
		pRS->m_min = x;
	if (x > pRS->m_max)
		pRS->m_max = x;
}

unsigned RS_NumDataValues(RunningStat *const pRS) // const
{
	return pRS->m_n;
}

float RS_Mean(RunningStat *const pRS) // const
{
	return (pRS->m_n > 0) ? pRS->m_newM : 0.0f;
}

float RS_Variance(RunningStat *const pRS) // const
{
	return ((pRS->m_n > 1) ? pRS->m_newS / (pRS->m_n - 1) : 0.0f);
}

float RS_StandardDeviation(RunningStat *const pRS) // const
{
	return sqrt(RS_Variance(pRS));
}

float RS_Min(RunningStat *const pRS) {
	return pRS->m_min;
}

float RS_Max(RunningStat *const pRS) {
	return pRS->m_max;
}

/* [] END OF FILE */
