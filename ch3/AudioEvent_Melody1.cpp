/*===================================================

AudioEvent_Melody1.cpp

Written by Alan Wolfe 6/2012

===================================================*/

#include "AudioEvent_Melody1.h"
#include "AudioEventMgr.h"
#include "oscillators.h"
#include "utils.h"

CAudioEvent_Melody1::CAudioEvent_Melody1(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime)
:CAudioEvent(pParentMgr, nStartSample, nEndSample, fEndTime)
,m_fPhase(0.0f)
,m_LeftVolumePhase(0.0f)
{
}

CAudioEvent_Melody1::~CAudioEvent_Melody1()
{
}

void CAudioEvent_Melody1::GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate)
{
	//calculate which quarter note we are on
	int nQuarterNote = (nRelativeBlock * 4) / nSampleRate;

	//make our melody
	float fFrequency = 0.0f;

	if(nQuarterNote < 16)
	{
		fFrequency = CalcFrequency(0,5);
	}
	else
	{
		nQuarterNote = (nQuarterNote - 16) % 32;

		if(nQuarterNote < 8)
		{
			fFrequency = CalcFrequency(0,10);
		}
		else if(nQuarterNote < 16)
		{
			fFrequency = CalcFrequency(0,12);
		}
		else if(nQuarterNote < 24)
		{
			fFrequency = CalcFrequency(0,7);
		}
		else
		{
			fFrequency = CalcFrequency(0,5);
		}
	}

	float fValue = AdvanceOscilator_Saw_BandLimited(m_fPhase,fFrequency,(float)m_pParentMgr->GetSampleRate(),4);

	//apply an opening envelope if we should
	float fRelativeTime = ((float)nRelativeBlock) / ((float)nSampleRate);
	float fEnvelopeVolume = 1.0f;
	if(fRelativeTime < 4.0f)
	{
		fEnvelopeVolume = fRelativeTime / 4.0f;

		//square it to help compensate for the fact that multiplying by 0.5 doesn't make something half as loud (perception of volume isn't linear!)
		//square it again to force it to be a sub linear curve to make it sound like non linear in the other direction
		fEnvelopeVolume = fEnvelopeVolume * fEnvelopeVolume * fEnvelopeVolume * fEnvelopeVolume;
	}
	fValue *= fEnvelopeVolume;
	
	
	//modulate the volume of the left and right channels on slow, offset sine waves for a fun effect!
	float fLeftVolume = AdvanceOscilator_Sine(m_LeftVolumePhase,0.15f,(float)m_pParentMgr->GetSampleRate());
	fLeftVolume = (fLeftVolume * 0.5f) + 0.5f;


	//apply the sound we generated
	pBlock[0] += fValue * fLeftVolume;

	//also to the right channel if there is one
	if(nNumChannels == 2)
	{
		pBlock[1] += fValue * (1.0f - fLeftVolume);
	}
}
