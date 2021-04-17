/*===================================================

wavefile.h

This handles reading and writing wave files

Written by Alan Wolfe 5/2012

http://demofox.org

some useful links about the wave file format:
http://www.piclist.com/techref/io/serial/midi/wave.html
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/

Note: This source code assumes that you are on a little endian machine.

===================================================*/

#ifndef WAVEFILE_H
#define WAVEFILE_H

#include "utils.h"

//explicit types to get around the templatized function compilation problem
bool WriteWaveFileUint8(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData = true);
bool WriteWaveFileInt16(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData = true);
bool WriteWaveFileInt32(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData = true);

bool ReadWaveFile(const char *szFileName, float *&pData, int32 &nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData = true);

#endif //WAVEFILE_H
