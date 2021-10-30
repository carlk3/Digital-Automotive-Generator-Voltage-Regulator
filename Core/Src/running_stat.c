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

#include <math.h>

#include <running_stat.h>

// public:
void RS_init(RunningStat *const this) {
	this->m_n = 0;
}

void RS_Clear(RunningStat *const this) {
	this->m_n = 0;
}

void RS_Push(RunningStat *const this, const float x) {
	this->m_n++;

	// See Knuth TAOCP vol 2, 3rd edition, page 232
	if (this->m_n == 1) {
		this->m_oldM = this->m_newM = x;
		this->m_oldS = 0.0;
	} else {
		this->m_newM = this->m_oldM + (x - this->m_oldM) / this->m_n;
		this->m_newS = this->m_oldS + (x - this->m_oldM) * (x - this->m_newM);

		// set up for next iteration
		this->m_oldM = this->m_newM;
		this->m_oldS = this->m_newS;
	}
}

unsigned RS_NumDataValues(RunningStat *const this) // const
{
	return this->m_n;
}

float RS_Mean(RunningStat *const this) // const
{
	return (this->m_n > 0) ? this->m_newM : 0.0f;
}

float RS_Variance(RunningStat *const this) // const
{
	return ((this->m_n > 1) ? this->m_newS / (this->m_n - 1) : 0.0f);
}

float RS_StandardDeviation(RunningStat *const this) // const
{
	return sqrt(RS_Variance(this));
}

/* [] END OF FILE */
