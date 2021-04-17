/*===================================================

oscillators.h

Standard oscillators

Written by Alan Wolfe 5/2012

===================================================*/

#ifndef OSCILLATORS_H
#define OSCILLATORS_H

float AdvanceOscilator_Sine(float &fPhase, float fFrequency, float fSampleRate);
float AdvanceOscilator_Saw(float &fPhase, float fFrequency, float fSampleRate);
float AdvanceOscilator_Square(float &fPhase, float fFrequency, float fSampleRate);
float AdvanceOscilator_Triangle(float &fPhase, float fFrequency, float fSampleRate);
float AdvanceOscilator_Noise(float &fPhase, float fFrequency, float fSampleRate, float fLastValue);

float AdvanceOscilator_Saw_BandLimited(float &fPhase, float fFrequency, float fSampleRate, int nNumHarmonics=0);
float AdvanceOscilator_Square_BandLimited(float &fPhase, float fFrequency, float fSampleRate, int nNumHarmonics=0);
float AdvanceOscilator_Triangle_BandLimited(float &fPhase, float fFrequency, float fSampleRate, int nNumHarmonics=0);

#endif //OSCILLATORS_H