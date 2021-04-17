/*===================================================

AudioEventMgr.h

A manager for audio events

Written by Alan Wolfe 6/2012

===================================================*/

#ifndef AUDIOEVENTMGR_H
#define AUDIOEVENTMGR_H

#include <vector>
using namespace std;

//we have to include the header instead of being able to forward declare these classes because the body of the templated AddEvent function
//has to be in the header due to how c++ works unfortuantely
#include "AudioEvent_PlayWav.h"

typedef vector<CAudioEvent *> AudioEventList;

class CAudioEventMgr
{
public:
	CAudioEventMgr(int nSampleRate, int nNumChannels, float fNumSeconds);
	~CAudioEventMgr();

	template <class AudioEventType>
	AudioEventType *AddEvent(float fStartTime = -1.0f, float fEndTime = -1.0f)
	{
		if(fStartTime < 0.0f)
			fStartTime = 0.0f;

		if(fEndTime < 0.0f)
			fEndTime = m_fNumSeconds;

		int nStartBlock = (int)(fStartTime * (float)m_nSampleRate);
		int nEndBlock = (int)(fEndTime * (float)m_nSampleRate);
		AudioEventType *pRet = new AudioEventType(this,nStartBlock,nEndBlock,fEndTime);
		m_AudioEvents.push_back(pRet);
		return pRet;
	}

	int GetNumChannels() const {return m_nNumChannels;}
	int GetSampleRate() const {return m_nSampleRate;}
	float GetGlobalTime() const {return m_fGlobalTime;}
	int GetGlobalBlock() const {return m_nGlobalBlock;}

	void RenderWaveFileUint8(const char *szFileName, bool bNormalizeData = true);
	void RenderWaveFileInt16(const char *szFileName, bool bNormalizeData = true);
	void RenderWaveFileInt32(const char *szFileName, bool bNormalizeData = true);
	void playWaveFileInt16();

private:
	void GetSample(float *pBlock, int nNumBlock, int nNumSample, int nSampleRate);
	void Render(void);

	AudioEventList m_AudioEvents;

	float *m_pRenderedData;
	int m_nNumSamples;

	int m_nSampleRate;
	int m_nNumChannels;
	float m_fNumSeconds;

	int m_nGlobalBlock;
	float m_fGlobalTime;
};

#endif //AUDIOEVENTMGR_H