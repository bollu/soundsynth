/*===================================================

AudioEvent.h

The base class for audio events

Written by Alan Wolfe 6/2012

===================================================*/

#pragma once

class CAudioEventMgr;

class CAudioEvent
{
public:
	CAudioEvent(CAudioEventMgr *pParentMgr, int nStartBlock, int nEndBlock, float fEndTime)
	:m_pParentMgr(pParentMgr)
	,m_nStartBlock(nStartBlock)
	,m_nEndBlock(nEndBlock)
	,m_fEndTime(fEndTime)
	{
	}

	virtual ~CAudioEvent() {}

	virtual void GetSample(float *pBlock, int nRelativeBlock, int nNumChannels, int nSampleRate) = 0;

	int GetStartBlock() const {return m_nStartBlock;}

	bool BlockInRange(int nBlock) const
	{
		return nBlock >= m_nStartBlock && nBlock < m_nEndBlock;
	}

protected:
	int m_nStartBlock;
	int m_nEndBlock;
	float m_fEndTime;
	CAudioEventMgr *m_pParentMgr;
};
