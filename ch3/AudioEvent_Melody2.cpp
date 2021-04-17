/*===================================================

AudioEvent_Melody2.cpp

Written by Alan Wolfe 6/2012

===================================================*/

#include "AudioEvent_Melody2.h"
#include "AudioEventMgr.h"
#include "oscillators.h"
#include "utils.h"

CAudioEvent_Melody2::CAudioEvent_Melody2(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime)
:CAudioEvent(pParentMgr, nStartSample, nEndSample, fEndTime)
,m_fPhase(0.0f)
{
}

CAudioEvent_Melody2::~CAudioEvent_Melody2()
{
}

void CAudioEvent_Melody2::GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate)
{
	//calculate which quarter note we are on
	int nEighthNote = (nRelativeBlock * 8) / nSampleRate;

	//make our melody
	float fFrequency = 0.0f;

	if((nEighthNote/32) % 2 == 0)
	{
		nEighthNote = nEighthNote % 4;

		switch(nEighthNote)
		{
			case 0:
			{
				fFrequency = CalcFrequency(2,5);
				break;
			}
			case 1:
			{
				fFrequency = CalcFrequency(2,7);
				break;
			}
			case 2:
			{
				fFrequency = CalcFrequency(2,12);
				break;
			}
			case 3:
			{
				fFrequency = CalcFrequency(2,10);
				break;
			}
		}
	}
	else
	{
		nEighthNote = nEighthNote % 4;

		switch(nEighthNote)
		{
			case 0:
			{
				fFrequency = CalcFrequency(2,7);
				break;
			}
			case 1:
			{
				fFrequency = CalcFrequency(2,9);
				break;
			}
			case 2:
			{
				fFrequency = CalcFrequency(2,14);
				break;
			}
			case 3:
			{
				fFrequency = CalcFrequency(2,12);
				break;
			}
		}
	}

	//make our current sample
	float fValue = AdvanceOscilator_Square_BandLimited(m_fPhase,fFrequency,(float)m_pParentMgr->GetSampleRate(), 3);

	//make it a lil quieter
	fValue *= 0.125f;

	//apply the sound we generated
	pBlock[0] += fValue;

	if(nNumChannels == 2)
	{
		pBlock[1] += fValue;
	}
}
