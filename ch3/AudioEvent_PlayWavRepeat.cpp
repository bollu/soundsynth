/*===================================================

AudioEvent_PlayWavRepeat.cpp

An audio event for playing wave files repeatedly (like for a drum beat)

Written by Alan Wolfe 6/2012

===================================================*/

#include "AudioEvent_PlayWavRepeat.h"
#include "wavefile.h"
#include "AudioEventMgr.h"

CAudioEvent_PlayWavRepeat::CAudioEvent_PlayWavRepeat(CAudioEventMgr *pParentMgr, int nStartSample, int nEndSample, float fEndTime)
:CAudioEvent(pParentMgr, nStartSample, nEndSample, fEndTime)
,m_pData(NULL)
,m_nNumSamples(0)
,m_nNumBlocks(0)
{
}

CAudioEvent_PlayWavRepeat::~CAudioEvent_PlayWavRepeat()
{
}

void CAudioEvent_PlayWavRepeat::GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate)
{
	if(nRelativeBlock == m_nRepeatBlocks)
	{
		m_nStartBlock = m_pParentMgr->GetGlobalBlock();
	}

	//adjust the block # by the playback rate
	nRelativeBlock = (int)((float)nRelativeBlock * m_fPlaybackRate);

	//if we are done playing the sound, bail
	if(nRelativeBlock > m_nNumBlocks)
	{
		return;
	}

	//left channel
	pBlock[0] += m_pData[nRelativeBlock * nNumChannels];

	//right channel if there is one
	if(nNumChannels == 2)
	{
		pBlock[1] += m_pData[nRelativeBlock * nNumChannels + 1];
	}
}

bool CAudioEvent_PlayWavRepeat::Init(const char *szFileName, float fPlaybackRate /*= 1.0f*/, float fRepeatTime /*= 1.0f*/)
{
	//store off the params
	m_fPlaybackRate = fPlaybackRate;
	m_nRepeatBlocks = (int)(fRepeatTime * (float)m_pParentMgr->GetSampleRate());

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