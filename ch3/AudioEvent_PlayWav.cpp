/*===================================================

AudioEvent_PlayWav.cpp

An audio event for playing wave files

Written by Alan Wolfe 6/2012

===================================================*/

#include "AudioEvent_PlayWav.h"
#include "wavefile.h"
#include "AudioEventMgr.h"

CAudioEvent_PlayWav::CAudioEvent_PlayWav(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime)
:CAudioEvent(pParentMgr, nStartSample, nEndSample, fEndTime)
,m_pData(NULL)
,m_nNumSamples(0)
,m_nNumBlocks(0)
{
}

CAudioEvent_PlayWav::~CAudioEvent_PlayWav()
{
}

void CAudioEvent_PlayWav::GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate)
{
	//adjust the block # by the playback rate and start time and make sure it's in range
	nRelativeBlock = (int)((float)(nRelativeBlock + m_nBlockOffset) * m_fPlaybackRate) % m_nNumBlocks;

	if(nRelativeBlock < 0)
		nRelativeBlock += m_nNumBlocks;

	//apply any fade out envelope out if there is one
	float fVolume = 1.0f;
	if(m_fEndTime - m_fFadeOutTime < m_pParentMgr->GetGlobalTime())
	{
		fVolume = (m_fEndTime - m_pParentMgr->GetGlobalTime()) / m_fFadeOutTime;
	}

	//left channel
	pBlock[0] += m_pData[nRelativeBlock * nNumChannels] * fVolume * m_fVolume;

	//right channel if there is one
	if(nNumChannels == 2)
	{
		pBlock[1] += m_pData[nRelativeBlock * nNumChannels + 1] * fVolume * m_fVolume;
	}
}

bool CAudioEvent_PlayWav::Init(const char *szFileName, float fVolume /*= 1.0f*/, float fStartTime /*= 0.0f*/, float fPlaybackRate /*= 1.0f*/, float fFadeOutTime /*= 0.0f*/)
{
	//store off the playback rate and start time
	m_nBlockOffset = (int)(fStartTime * (float)m_pParentMgr->GetSampleRate());
	m_fPlaybackRate = fPlaybackRate;
	m_fFadeOutTime = fFadeOutTime;
	m_fVolume = fVolume;

	//make sure there is not any existing audio data
	delete[] m_pData;

	//read in our wave file if we can
	if(!ReadWaveFile(szFileName,m_pData,m_nNumSamples,m_pParentMgr->GetNumChannels(),m_pParentMgr->GetSampleRate()))
	{
		return false;
	}

	//calculate how many blocks there are
	m_nNumBlocks = m_nNumSamples / m_pParentMgr->GetNumChannels();

	return true;
}