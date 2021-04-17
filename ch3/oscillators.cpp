/*===================================================

oscillators.cpp

Standard oscillators

Written by Alan Wolfe 5/2012

===================================================*/

#include "oscillators.h"
#include "utils.h"
#include <stdlib.h>

//pass in the current phase, frequency, and sample rate, and these tone generation functions
//will advance the phase and return the sample for that phase

float AdvanceOscilator_Sine(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += 2 * (float)M_PI * fFrequency/fSampleRate;

	while(fPhase >= 2 * (float)M_PI)
		fPhase -= 2 * (float)M_PI;

	while(fPhase < 0)
		fPhase += 2 * (float)M_PI;

	return sin(fPhase);
}

float AdvanceOscilator_Saw(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += fFrequency/fSampleRate;

	while(fPhase > 1.0f)
		fPhase -= 1.0f;

	while(fPhase < 0.0f)
		fPhase += 1.0f;

	return ((fPhase * 2.0f) - 1.0f)*-1.0f;
}

float AdvanceOscilator_Square(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += fFrequency/fSampleRate;

	while(fPhase > 1.0f)
		fPhase -= 1.0f;

	while(fPhase < 0.0f)
		fPhase += 1.0f;

	if(fPhase <= 0.5f)
		return 1.0f;
	else
		return -1.0f;
}

float AdvanceOscilator_Triangle(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += fFrequency/fSampleRate;

	while(fPhase > 1.0f)
		fPhase -= 1.0f;

	while(fPhase < 0.0f)
		fPhase += 1.0f;

	float fRet;
	if(fPhase <= 0.5f)
		fRet=fPhase*2;
	else
		fRet=(1.0f - fPhase)*2;

	return (fRet * 2.0f) - 1.0f;
}

float AdvanceOscilator_Noise(float &fPhase, float fFrequency, float fSampleRate, float fLastValue)
{
	unsigned int nLastSeed = (unsigned int)fPhase;
	fPhase += fFrequency/fSampleRate;
	unsigned int nSeed = (unsigned int)fPhase;

	while(fPhase > 2.0f)
		fPhase -= 1.0f;

	if(nSeed != nLastSeed)
	{
		float fValue = ((float)rand()) / ((float)RAND_MAX);
		fValue = (fValue * 2.0f) - 1.0f;

		//uncomment the below to make it slightly more intense
		/*
		if(fValue < 0)
			fValue = -1.0f;
		else
			fValue = 1.0f;
		*/

		return fValue;
	}
	else
	{
		return fLastValue;
	}
}

float AdvanceOscilator_Saw_BandLimited(float &fPhase, float fFrequency, float fSampleRate, int nNumHarmonics /*=0*/)
{
	//advance the phase
	fPhase += 2 * (float)M_PI * fFrequency/fSampleRate;

	while(fPhase >= 2 * (float)M_PI)
		fPhase -= 2 * (float)M_PI;

	while(fPhase < 0)
		fPhase += 2 * (float)M_PI;

	//if num harmonics is zero, calculate how many max harmonics we can do
	//without going over the nyquist frequency (half of sample rate frequency)
	if(nNumHarmonics == 0 && fFrequency != 0.0f)
	{
		float fTempFreq = fFrequency;

		while(fTempFreq < fSampleRate * 0.5f)
		{
			nNumHarmonics++;
			fTempFreq *= 2.0f;
		}
	}

	//calculate the saw wave sample
	float fRet = 0.0f;
	for(int nHarmonicIndex = 1; nHarmonicIndex <= nNumHarmonics; ++nHarmonicIndex)
	{
		fRet+=sin(fPhase*(float)nHarmonicIndex)/(float)nHarmonicIndex;
	}

	//adjust the volume
	fRet = fRet * 2.0f / (float)M_PI;

	return fRet;
}

float AdvanceOscilator_Square_BandLimited(float &fPhase, float fFrequency, float fSampleRate, int nNumHarmonics/*=0*/)
{
	//advance the phase
	fPhase += 2 * (float)M_PI * fFrequency/fSampleRate;

	while(fPhase >= 2 * (float)M_PI)
		fPhase -= 2 * (float)M_PI;

	while(fPhase < 0)
		fPhase += 2 * (float)M_PI;

	//if num harmonics is zero, calculate how many max harmonics we can do
	//without going over the nyquist frequency (half of sample rate frequency)
	if(nNumHarmonics == 0 && fFrequency != 0.0f)
	{
		while(fFrequency * (float)(nNumHarmonics*2-1) < fSampleRate * 0.5f)
		{
			nNumHarmonics++;
		}

		nNumHarmonics--;
	}

	//calculate the square wave sample
	float fRet = 0.0f;
	for(int nHarmonicIndex = 1; nHarmonicIndex <= nNumHarmonics; ++nHarmonicIndex)
	{
		fRet+=sin(fPhase*(float)(nHarmonicIndex*2-1))/(float)(nHarmonicIndex*2-1);
	}

	//adjust the volume
	fRet = fRet * 4.0f / (float)M_PI;

	return fRet;
}

float AdvanceOscilator_Triangle_BandLimited(float &fPhase, float fFrequency, float fSampleRate, int nNumHarmonics/*=0*/)
{
	//advance the phase
	fPhase += 2 * (float)M_PI * fFrequency/fSampleRate;

	while(fPhase >= 2 * (float)M_PI)
		fPhase -= 2 * (float)M_PI;

	while(fPhase < 0)
		fPhase += 2 * (float)M_PI;

	//if num harmonics is zero, calculate how many max harmonics we can do
	//without going over the nyquist frequency (half of sample rate frequency)
	if(nNumHarmonics == 0 && fFrequency != 0.0f)
	{
		while(fFrequency * (float)(nNumHarmonics*2-1) < fSampleRate * 0.5f)
		{
			nNumHarmonics++;
		}

		nNumHarmonics--;
	}

	//calculate the triangle wave sample
	bool bSubtract = true;
	float fRet = 0.0f;
	for(int nHarmonicIndex = 1; nHarmonicIndex <= nNumHarmonics; ++nHarmonicIndex)
	{
		if(bSubtract)
			fRet-=sin(fPhase*(float)(nHarmonicIndex*2-1))/((float)(nHarmonicIndex*2-1) * (float)(nHarmonicIndex*2-1));
		else
			fRet+=sin(fPhase*(float)(nHarmonicIndex*2-1))/((float)(nHarmonicIndex*2-1) * (float)(nHarmonicIndex*2-1));

		bSubtract = !bSubtract;
	}

	//adjust the volume
	fRet = fRet * 8.0f / ((float)M_PI * (float)M_PI);

	return fRet;
}