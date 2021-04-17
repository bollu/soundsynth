// https://blog.demofox.org/2012/05/14/diy-synthesizer-chapter-1-sound-output/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

using uint32 = uint32_t;
using uint16 = uint16_t;
using int32 = int32_t;
using int16 = int16_t;

float CalcFrequency(float fOctave,float fNote)
{

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
    return (float)(440*pow(2.0,((double)((fOctave-4)*12+fNote))/12.0));
}

// this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader {
    // the main chunk
    unsigned char m_szChunkID[4];
    uint32 m_nChunkSize;
    unsigned char m_szFormat[4];

    // sub chunk 1 "fmt "
    unsigned char m_szSubChunk1ID[4];
    uint32 m_nSubChunk1Size;
    uint16 m_nAudioFormat;
    uint16 m_nNumChannels;
    uint32 m_nSampleRate;
    uint32 m_nByteRate;
    uint16 m_nBlockAlign;
    uint16 m_nBitsPerSample;

    // sub chunk 2 "data"
    unsigned char m_szSubChunk2ID[4];
    uint32 m_nSubChunk2Size;

    // then comes the data!
};

bool WriteWaveFile(const char *szFileName, void *pData, int32 nDataSize,
                   int16 nNumChannels, int32 nSampleRate,
                   int32 nBitsPerSample) {
    // open the file if we can
    FILE *File = fopen(szFileName, "w+b");
    if (!File) {
        return false;
    }

    SMinimalWaveFileHeader waveHeader;

    // fill out the main chunk
    memcpy(waveHeader.m_szChunkID, "RIFF", 4);
    waveHeader.m_nChunkSize = nDataSize + 36;
    memcpy(waveHeader.m_szFormat, "WAVE", 4);

    // fill out sub chunk 1 "fmt "
    memcpy(waveHeader.m_szSubChunk1ID, "fmt ", 4);
    waveHeader.m_nSubChunk1Size = 16;
    waveHeader.m_nAudioFormat = 1;
    waveHeader.m_nNumChannels = nNumChannels;
    waveHeader.m_nSampleRate = nSampleRate;
    waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBitsPerSample = nBitsPerSample;

    // fill out sub chunk 2 "data"
    memcpy(waveHeader.m_szSubChunk2ID, "data", 4);
    waveHeader.m_nSubChunk2Size = nDataSize;

    // write the header
    fwrite(&waveHeader, sizeof(SMinimalWaveFileHeader), 1, File);

    // write the wave data itself
    fwrite(pData, nDataSize, 1, File);

    // close the file and return success
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
    for (int i = 0; i < nNumSamples; ++i) {
        nValue += 8000000;
        pData[i] = nValue;
    }

    WriteWaveFile("ch1-outmono.wav", pData, nNumSamples * sizeof(pData[0]),
                  nNumChannels, nSampleRate, sizeof(pData[0]) * 8);
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
    for (int i = 0; i < nNumSamples; i += 2) {
        nValue1 += 4000000;
        nValue2 += 12000000;
        pData[i] = nValue1;      // left channel
        pData[i + 1] = nValue2;  // right channel
    }
    WriteWaveFile("ch1-outstereo.wav", pData, nNumSamples * sizeof(pData[0]),
                  nNumChannels, nSampleRate, sizeof(pData[0]) * 8);

    delete[] pData;
}

void sin() {
    int nSampleRate = 44100;
    int nNumSeconds = 4;
    int nNumChannels = 1;
    int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
    int32 *pData = new int32[nNumSamples];
    // make a naive sine wave
    float fFrequency = CalcFrequency(3,3);
    for (int i = 0; i < nNumSamples; ++i) {
        pData[i] = 3e8 * sin((float)i * 2 * (float)M_PI * fFrequency /
                            (float)nSampleRate);
    }
    WriteWaveFile("ch2-sinenaive.wav", pData, nNumSamples * sizeof(pData[0]), 
            nNumChannels, nSampleRate, sizeof(pData[0]) * 8);
}

int main() {
    mono();
    stereo();
    sin();
    return 0;
}
