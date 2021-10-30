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
	float m_oldM, m_newM, m_oldS, m_newS;
} RunningStat;

void RS_init(RunningStat *const this);
void RS_Clear(RunningStat *const this);
void RS_Push(RunningStat *const this, const float x);
unsigned RS_NumDataValues(RunningStat *const this);
float RS_Mean(RunningStat *const this);
float RS_Variance(RunningStat *const this);
float RS_StandardDeviation(RunningStat *const this);

#endif
/* [] END OF FILE */
