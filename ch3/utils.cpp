/*===================================================

utils.h

Written by Alan Wolfe 5/2012

http://demofox.org

===================================================*/

#include "utils.h"

//calculate the frequency of the specified note.
//fractional notes allowed!
float CalcFrequency(float fOctave,float fNote)
/*
	Calculate the frequency of any note!
	frequency = 440×(2^(n/12))

	N=0 is A4
	N=1 is A#4
	etc...

	notes go like so...
	0  = A
	1  = A#
	2  = B
	3  = C
	4  = C#
	5  = D
	6  = D#
	7  = E
	8  = F
	9  = F#
	10 = G
	11 = G#
*/
{
	return (float)(440*pow(2.0,((double)((fOctave-4)*12+fNote))/12.0));
}

void NormalizeAudioData(float *pData,int nNumSamples)
{
	//figure out what the maximum and minimum value is
	float fMaxValue = pData[0];
	float fMinValue = pData[0];
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		if(pData[nIndex] > fMaxValue)
		{
			fMaxValue = pData[nIndex];
		}

		if(pData[nIndex] < fMinValue)
		{
			fMinValue = pData[nIndex];
		}
	}
	
	//calculate the center and the height
	float fCenter = (fMinValue + fMaxValue) / 2.0f;
	float fHeight = fMaxValue - fMinValue;

	//center and normalize the samples
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		//center the samples
		pData[nIndex] -= fCenter;

		//normalize the samples
		pData[nIndex] /= fHeight;
	}
}

float GetLerpedAudioSample(float *pData, int nNumSamples, float fIndex)
{
	int nIndex1 = (int)fIndex;
	int nIndex2 = nIndex1 + 1;

	if(nIndex1 < 0)
		nIndex1 = 0;

	if(nIndex2 < 0)
		nIndex2 = 0;

	if(nIndex1 >= nNumSamples)
		nIndex1 = nNumSamples - 1;

	if(nIndex2 >= nNumSamples)
		nIndex2 = nNumSamples - 1;

	float fLerp = fmodf(fIndex,1.0f);

	float fSample1 = pData[nIndex1];
	float fSample2 = pData[nIndex2];

	return ((fSample2 - fSample1) * fLerp) + fSample1;
}

void ResampleData(float *&pData, int &nNumSamples, int nSrcSampleRate, int nDestSampleRate)
{
	//if the requested sample rate is the sample rate it already is, bail out and do nothing
	if(nSrcSampleRate == nDestSampleRate)
	{
		return;
	}

	//calculate the ratio of the old sample rate to the new
	float fResampleRatio = (float)nDestSampleRate / (float) nSrcSampleRate;
	
	//calculate how many samples the new data will have and allocate the new sample data
	int nNewDataNumSamples = (int)((float)nNumSamples * fResampleRatio);
	float *pNewData = new float[nNewDataNumSamples];

	//get each lerped output sample.  There are higher quality ways to resample
	for(int nIndex = 0; nIndex < nNewDataNumSamples; ++nIndex)
	{
		pNewData[nIndex] = GetLerpedAudioSample(pData,nNumSamples,(float)nIndex / fResampleRatio);
	}

	//free the old data and set the new data
	delete[] pData;
	pData = pNewData;
	nNumSamples = nNewDataNumSamples;
}

void ChangeNumChannels(float *&pData, int &nNumSamples, int nSrcChannels, int nDestChannels)
{
	//if the number of channels requested is the number of channels already there, or either number of channels is not mono or stereo, return
	if(nSrcChannels == nDestChannels ||
	   nSrcChannels < 1 || nSrcChannels > 2 ||
	   nDestChannels < 1 || nDestChannels > 2)
	{
		return;
	}

	//if converting to stereo
	if(nDestChannels == 2)
	{
		float *pNewData = new float[nNumSamples * 2];
		for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
		{
			pNewData[nIndex * 2] = pData[nIndex];
			pNewData[nIndex * 2+1] = pData[nIndex];
		}

		delete pData;
		pData = pNewData;
		nNumSamples *= 2;
	}
	//else converting to mono
	else
	{
		float *pNewData = new float[nNumSamples / 2];
		for(int nIndex = 0; nIndex < nNumSamples / 2; ++nIndex)
		{
			pNewData[nIndex] = pData[nIndex * 2] + pData[nIndex * 2 + 1];
		}

		delete pData;
		pData = pNewData;
		nNumSamples /= 2;
	}
}