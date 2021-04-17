/*===================================================

utils.h

Written by Alan Wolfe 5/2012

http://demofox.org

===================================================*/

#ifndef UTILS_H
#define UTILS_H

#define _USE_MATH_DEFINES
#include <math.h>

//define our types.  If your environment varies, you can change these types to be what they should be
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

//Macros
#define CLAMP(value,min,max) {if(value < min) { value = min; } else if(value > max) { value = max; }}

//Functions
float CalcFrequency(float fOctave,float fNote);
void NormalizeAudioData(float *pData,int nNumSamples);
void ResampleData(float *&pData, int &nNumSamples, int nSrcSampleRate, int nDestSampleRate);
void ChangeNumChannels(float *&pData, int &nNumSamples, int nSrcChannels, int nDestChannels);

#endif //UTILS_H