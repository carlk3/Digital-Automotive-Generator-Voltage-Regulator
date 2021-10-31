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

#ifndef _RUNNING_STAT_H_
#define _RUNNING_STAT_H_

typedef struct {
// private:
	unsigned m_n;
	float m_oldM, m_newM, m_oldS, m_newS, m_min, m_max;
} RunningStat;

void RS_init(RunningStat *const pRS);
void RS_Clear(RunningStat *const pRS);
void RS_Push(RunningStat *const pRS, const float x);
unsigned RS_NumDataValues(RunningStat *const pRS);
float RS_Mean(RunningStat *const pRS);
float RS_Variance(RunningStat *const pRS);
float RS_StandardDeviation(RunningStat *const pRS);
float RS_Min(RunningStat *const pRS);
float RS_Max(RunningStat *const pRS);

#endif
/* [] END OF FILE */
