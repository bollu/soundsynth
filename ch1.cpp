// https://blog.demofox.org/2012/05/14/diy-synthesizer-chapter-1-sound-output/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

using uint32 = uint32_t;
using uint16 = uint16_t;
using int32 = int32_t;
using int16 = int16_t;

//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
  //the main chunk
  unsigned char m_szChunkID[4];
  uint32 m_nChunkSize;
  unsigned char m_szFormat[4];

  //sub chunk 1 "fmt "
  unsigned char m_szSubChunk1ID[4];
  uint32 m_nSubChunk1Size;
  uint16 m_nAudioFormat;
  uint16 m_nNumChannels;
  uint32 m_nSampleRate;
  uint32 m_nByteRate;
  uint16 m_nBlockAlign;
  uint16 m_nBitsPerSample;

  //sub chunk 2 "data"
  unsigned char m_szSubChunk2ID[4];
  uint32 m_nSubChunk2Size;

  //then comes the data!
};

bool WriteWaveFile(const char *szFileName,
		   void *pData,
		   int32 nDataSize,
		   int16 nNumChannels,
		   int32 nSampleRate,
		   int32 nBitsPerSample) {
  //open the file if we can
  FILE *File = fopen(szFileName,"w+b");
  if(!File) { return false; }

  SMinimalWaveFileHeader waveHeader;

  //fill out the main chunk
  memcpy(waveHeader.m_szChunkID,"RIFF",4);
  waveHeader.m_nChunkSize = nDataSize + 36;
  memcpy(waveHeader.m_szFormat,"WAVE",4);

  //fill out sub chunk 1 "fmt "
  memcpy(waveHeader.m_szSubChunk1ID,"fmt ",4);
  waveHeader.m_nSubChunk1Size = 16;
  waveHeader.m_nAudioFormat = 1;
  waveHeader.m_nNumChannels = nNumChannels;
  waveHeader.m_nSampleRate = nSampleRate;
  waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
  waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
  waveHeader.m_nBitsPerSample = nBitsPerSample;

  //fill out sub chunk 2 "data"
  memcpy(waveHeader.m_szSubChunk2ID,"data",4);
  waveHeader.m_nSubChunk2Size = nDataSize;

  //write the header
  fwrite(&waveHeader,sizeof(SMinimalWaveFileHeader),1,File);

  //write the wave data itself
  fwrite(pData,nDataSize,1,File);

  //close the file and return success
  fclose(File);
  return true;
}


void mono() {
  int nSampleRate = 44100;
  int nNumSeconds = 4;
  int nNumChannels = 1;
  int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
  int32 *pData = new int32[nNumSamples];

  int32 nValue = 0;
  for(int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
    nValue += 8000000;
    pData[nIndex] = nValue;
  }

  WriteWaveFile("ch1-outmono.wav",
		pData,
		nNumSamples * sizeof(pData[0]),
		nNumChannels,
		nSampleRate,
		sizeof(pData[0])*8);
  delete[] pData;
}

void stereo() {
  int nSampleRate = 44100;
  int nNumSeconds = 4;
  int nNumChannels = 1;
  int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
  int32 *pData = new int32[nNumSamples];

  int32 nValue1 = 0;
  int32 nValue2 = 0;
  for(int nIndex = 0; nIndex < nNumSamples; nIndex += 2) {
      nValue1 += 4000000;
      nValue2 += 12000000;
      pData[nIndex] = nValue1; //left channel
      pData[nIndex+1] = nValue2; //right channel
  }
  WriteWaveFile("ch1-outstereo.wav",
		pData,
		nNumSamples * sizeof(pData[0]),
		nNumChannels,
		nSampleRate,
		sizeof(pData[0])*8);
  
  delete[] pData;
}

int main() {
  mono();
  stereo();
  return 0;
}
