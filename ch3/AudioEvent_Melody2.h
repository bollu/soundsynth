/*===================================================

AudioEvent_Melody2.h

Written by Alan Wolfe 6/2012

===================================================*/

#ifndef AUDIOEVENT_MELODY2_H
#define AUDIOEVENT_MELODY2_H

#include "AudioEvent.h"

class CAudioEvent_Melody2 : public CAudioEvent
{
public:
	CAudioEvent_Melody2(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime);
	virtual ~CAudioEvent_Melody2();

	virtual void GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate);

private:
	float m_fPhase;
};

#endif //CAudioEvent_Melody2