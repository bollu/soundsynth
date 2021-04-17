/*===================================================

AudioEvent_PlayWav.h

An audio event for playing wave files

Written by Alan Wolfe 6/2012

===================================================*/

#ifndef AUDIOEVENT_PLAYWAV_H
#define AUDIOEVENT_PLAYWAV_H

#include "AudioEvent.h"

class CAudioEvent_PlayWav : public CAudioEvent
{
public:
	CAudioEvent_PlayWav(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime);
	virtual ~CAudioEvent_PlayWav();

	virtual void GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate);

	bool Init(const char *szFileName, float fVolume = 1.0f, float fStartTime = 0.0f, float fPlaybackRate = 1.0f, float fFadeOutTime = 0.0f);

private:
	float *m_pData;
	int m_nNumSamples;
	int m_nNumBlocks;
	float m_fPlaybackRate;
	int m_nBlockOffset;
	float m_fFadeOutTime;
	float m_fVolume;
};

#endif //AUDIOEVENT_PLAYWAV_H