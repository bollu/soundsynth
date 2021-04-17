/*===================================================

AudioEvent_Melody3.cpp

Written by Alan Wolfe 6/2012

===================================================*/

#include "AudioEvent_Melody3.h"
#include "AudioEventMgr.h"
#include "oscillators.h"
#include "utils.h"

CAudioEvent_Melody3::CAudioEvent_Melody3(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime)
:CAudioEvent(pParentMgr, nStartSample, nEndSample, fEndTime)
,m_fPhase(0.0f)
{
}

CAudioEvent_Melody3::~CAudioEvent_Melody3()
{
}

void CAudioEvent_Melody3::GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate)
{
	//calculate which quarter note we are on
	int nHalfNote = (nRelativeBlock * 4) / nSampleRate;

	float fFrequency = 0.0f;

	if((nHalfNote / 16) % 2 == 0)
	{
		nHalfNote = nHalfNote % 4;

		switch(nHalfNote)
		{
			case 0:
			{
				fFrequency = CalcFrequency(3,7);
				break;
			}
			case 1:
			{
				fFrequency = CalcFrequency(3,5);
				break;
			}
			case 2:
			{
				fFrequency = CalcFrequency(3,12);
				break;
			}
			case 3:
			{
				fFrequency = CalcFrequency(3,10);
				break;
			}
		}
	}
	else
	{
		bool bDoDifferent = (nHalfNote / 8) % 8 == 7;

		nHalfNote /= 2;
		nHalfNote = nHalfNote % 4;

		if(bDoDifferent)
		{
			switch(nHalfNote)
			{
				case 0:
				{
					fFrequency = CalcFrequency(3,12);
					break;
				}
				case 1:
				{
					fFrequency = CalcFrequency(3,10);
					break;
				}
				case 2:
				{
					fFrequency = CalcFrequency(3,5);
					break;
				}
				case 3:
				{
					fFrequency = CalcFrequency(3,7);
					break;
				}
			}
		}
		else
		{
			switch(nHalfNote)
			{
				case 0:
				{
					fFrequency = CalcFrequency(3,7);
					break;
				}
				case 1:
				{
					fFrequency = CalcFrequency(3,5);
					break;
				}
				case 2:
				{
					fFrequency = CalcFrequency(3,12);
					break;
				}
				case 3:
				{
					fFrequency = CalcFrequency(3,10);
					break;
				}
			}
		}
	}

	//make our current sample
	float fValue = AdvanceOscilator_Sine(m_fPhase,fFrequency,(float)m_pParentMgr->GetSampleRate());

	//make it a lil quieter
	fValue *= 0.25f;

	//apply the sound we generated
	pBlock[0] += fValue;

	if(nNumChannels == 2)
	{
		pBlock[1] += fValue;
	}
}
