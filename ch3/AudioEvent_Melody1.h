/*===================================================

AudioEvent_Melody1.h

Written by Alan Wolfe 6/2012

===================================================*/

#ifndef AUDIOEVENT_MELODY1_H
#define AUDIOEVENT_MELODY1_H

#include "AudioEvent.h"

class CAudioEvent_Melody1 : public CAudioEvent
{
public:
	CAudioEvent_Melody1(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime);
	virtual ~CAudioEvent_Melody1();

	virtual void GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate);

private:
	float m_fPhase;
	float m_LeftVolumePhase;
};

#endif //AUDIOEVENT_MELODY1_H