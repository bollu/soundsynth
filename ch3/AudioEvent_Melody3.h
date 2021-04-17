/*===================================================

AudioEvent_Melody3.h

Written by Alan Wolfe 6/2012

===================================================*/

#ifndef AUDIOEVENT_MELODY3_H
#define AUDIOEVENT_MELODY3_H

#include "AudioEvent.h"

class CAudioEvent_Melody3 : public CAudioEvent
{
public:
	CAudioEvent_Melody3(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime);
	virtual ~CAudioEvent_Melody3();

	virtual void GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate);

private:
	float m_fPhase;
};

#endif //CAudioEvent_Melody3