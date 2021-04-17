/*===================================================

AudioEvent_PlayWavRepeat.h

An audio event for playing wave files repeatedly (like for a drum beat)

Written by Alan Wolfe 6/2012

===================================================*/

#ifndef AUDIOEVENT_PLAYWAVREPEAT_H
#define AUDIOEVENT_PLAYWAVREPEAT_H

#include "AudioEvent.h"

class CAudioEvent_PlayWavRepeat : public CAudioEvent
{
public:
	CAudioEvent_PlayWavRepeat(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime);
	virtual ~CAudioEvent_PlayWavRepeat();

	virtual void GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate);

	bool Init(const char *szFileName, float fPlaybackRate = 1.0f, float fRepeatTime = 1.0f);

private:
	float *m_pData;
	int m_nNumSamples;
	int m_nNumBlocks;

	float m_fPlaybackRate;
	int m_nRepeatBlocks;
};

#endif //AUDIOEVENT_PLAYWAVREPEAT_H