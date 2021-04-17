# Kavariance

`bollu` learns digital audio synthesis from [the blog at the bottom of the sea](https://blog.demofox.org/2012/05/14/diy-synthesizer-chapter-1-sound-output/).
the name is a pun on [Kaveri](https://en.wikipedia.org/wiki/Kaveri), A `wav`/river goddess,
and [Covariance](https://en.wikipedia.org/wiki/Covariance)

#### ch2 code

```cpp
/*===================================================
 
Written by Alan Wolfe 5/2012
 
http://demofox.org
 
some useful links about the wave file format:
http://www.piclist.com/techref/io/serial/midi/wave.html
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 
Note: This source code assumes that you are on a little endian machine.
 
===================================================*/
 
#include 
#include 
#include 
 
#define _USE_MATH_DEFINES
#include 
 
//define our types.  If your environment varies, you can change these types to be what they should be
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;
 
//some macros
#define CLAMP(value,min,max) {if(value  max) { value = max; }}
 
//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
    //the main chunk
    unsigned char m_szChunkID[4];
    uint32        m_nChunkSize;
    unsigned char m_szFormat[4];
 
    //sub chunk 1 "fmt "
    unsigned char m_szSubChunk1ID[4];
    uint32        m_nSubChunk1Size;
    uint16        m_nAudioFormat;
    uint16        m_nNumChannels;
    uint32        m_nSampleRate;
    uint32        m_nByteRate;
    uint16        m_nBlockAlign;
    uint16        m_nBitsPerSample;
 
    //sub chunk 2 "data"
    unsigned char m_szSubChunk2ID[4];
    uint32        m_nSubChunk2Size;
 
    //then comes the data!
};
 
//0 to 255
void ConvertFloatToAudioSample(float fFloat, uint8 &nOut)
{
    fFloat = (fFloat + 1.0f) * 127.5f;
    CLAMP(fFloat,0.0f,255.0f);
    nOut = (uint8)fFloat;
}
 
//–32,768 to 32,767
void ConvertFloatToAudioSample(float fFloat, int16 &nOut)
{
    fFloat *= 32767.0f;
    CLAMP(fFloat,-32768.0f,32767.0f);
    nOut = (int16)fFloat;
}
 
//–2,147,483,648 to 2,147,483,647
void ConvertFloatToAudioSample(float fFloat, int32 &nOut)
{
    double dDouble = (double)fFloat;
    dDouble *= 2147483647.0;
    CLAMP(dDouble,-2147483648.0,2147483647.0);
    nOut = (int32)dDouble;
}
 
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
 
 
//this writes a wave file
//specify sample bit count as the template parameter
//can be uint8, int16 or int32
template
bool WriteWaveFile(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate)
{
    //open the file if we can
    FILE *File = fopen(szFileName,"w+b");
    if(!File)
    {
        return false;
    }
 
    //calculate bits per sample and the data size
    int32 nBitsPerSample = sizeof(T) * 8;
    int nDataSize = nNumSamples * sizeof(T);
 
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
 
    //write the wave data itself, converting it from float to the type specified
    T *pData = new T[nNumSamples];
    for(int nIndex = 0; nIndex = 2 * (float)M_PI)
        fPhase -= 2 * (float)M_PI;
 
    while(fPhase  1.0f)
        fPhase -= 1.0f;
 
    while(fPhase  1.0f)
        fPhase -= 1.0f;
 
    while(fPhase < 0.0f)
        fPhase += 1.0f;
 
    if(fPhase  1.0f)
        fPhase -= 1.0f;
 
    while(fPhase < 0.0f)
        fPhase += 1.0f;
 
    float fRet;
    if(fPhase  2.0f)
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
 
//the entry point of our application
int main(int argc, char **argv)
{
    //our parameters that all the wave forms use
    int nSampleRate = 44100;
    int nNumSeconds = 4;
    int nNumChannels = 1;
    float fFrequency = CalcFrequency(3,3); // middle C
 
    //make our buffer to hold the samples
    int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
    float *pData = new float[nNumSamples];
 
    //the phase of our oscilator, we don't really need to reset it between wave files
    //it just needs to stay continuous within a wave file
    float fPhase = 0;
 
    //make a naive sine wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = sin((float)nIndex * 2 * (float)M_PI * fFrequency / (float)nSampleRate);
    }
 
    WriteWaveFile("sinenaive.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a discontinuitous (popping) sine wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        if(nIndex < nNumSamples / 2)
        {
            float fCurrentFrequency = CalcFrequency(3,3);
            pData[nIndex] = sin((float)nIndex * 2 * (float)M_PI * fCurrentFrequency / (float)nSampleRate);
        }
        else
        {
            float fCurrentFrequency = CalcFrequency(3,4);
            pData[nIndex] = sin((float)nIndex * 2 * (float)M_PI * fCurrentFrequency / (float)nSampleRate);
        }
    }
 
    WriteWaveFile("sinediscon.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a continuous sine wave that changes frequencies
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        if(nIndex = 2 * (float)M_PI)
                fPhase -= 2 * (float)M_PI;
 
            while(fPhase = 2 * (float)M_PI)
                fPhase -= 2 * (float)M_PI;
 
            while(fPhase < 0)
                fPhase += 2 * (float)M_PI;
 
            pData[nIndex] = sin(fPhase);
        }
    }
 
    WriteWaveFile("sinecon.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a sine wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = AdvanceOscilator_Sine(fPhase,fFrequency,(float)nSampleRate);
    }
 
    WriteWaveFile("sine.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a quieter sine wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = AdvanceOscilator_Sine(fPhase,fFrequency,(float)nSampleRate) * 0.4f;
    }
 
    WriteWaveFile("sinequiet.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a clipping sine wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = AdvanceOscilator_Sine(fPhase,fFrequency,(float)nSampleRate) * 1.4f;
    }
 
    WriteWaveFile("sineclip.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a saw wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = AdvanceOscilator_Saw(fPhase,fFrequency,(float)nSampleRate);
    }
 
    WriteWaveFile("saw.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a square wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = AdvanceOscilator_Square(fPhase,fFrequency,(float)nSampleRate);
    }
 
    WriteWaveFile("square.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a triangle wave
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        pData[nIndex] = AdvanceOscilator_Triangle(fPhase,fFrequency,(float)nSampleRate);
    }
 
    WriteWaveFile("triangle.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make some noise or... make... some... NOISE!!!
    for(int nIndex = 0; nIndex  0 ? pData[nIndex-1] : 0.0f);
    }
 
    WriteWaveFile("noise.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //make a dumb little song
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
    {
        //calculate which quarter note we are on
        int nQuarterNote = nIndex * 4 / nSampleRate;
        float fQuarterNotePercent = (float)((nIndex * 4) % nSampleRate) / (float)nSampleRate;
 
        //intentionally add a "pop" noise mid way through the 3rd quarter note
        if(nIndex == nSampleRate * 3 / 4 + nSampleRate / 8)
        {
            pData[nIndex] = -1.0f;
            continue;
        }
 
        //do different logic based on which quarter note we are on
        switch(nQuarterNote)
        {
            case 0:
            {
                pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,0),(float)nSampleRate);
                break;
            }
            case 1:
            {
                pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,2),(float)nSampleRate);
                break;
            }
            case 2:
            case 3:
            {
                pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,5),(float)nSampleRate);
                break;
            }
            case 4:
            {
                pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate);
                break;
            }
            case 5:
            {
                pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,3 + fQuarterNotePercent * 2.0f),(float)nSampleRate);
                break;
            }
            case 6:
            {
                pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate) * (1.0f - fQuarterNotePercent);
                break;
            }
 
            case 8:
            {
                pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,0),(float)nSampleRate);
                break;
            }
            case 9:
            {
                pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,2),(float)nSampleRate);
                break;
            }
            case 10:
            case 11:
            {
                pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,5),(float)nSampleRate);
                break;
            }
            case 12:
            {
                pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate);
                break;
            }
            case 13:
            {
                pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,3 + fQuarterNotePercent * 2.0f),(float)nSampleRate);
                break;
            }
            case 14:
            {
                pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate) * (1.0f - fQuarterNotePercent);
                break;
            }
 
            default:
            {
                pData[nIndex] = 0;
                break;
            }
        }
    }
 
    WriteWaveFile("song.wav",pData,nNumSamples,nNumChannels,nSampleRate);
 
    //free our data buffer
    delete[] pData;
}
```

#### ch3 code

#### plucked string: karplus strong

- [HN discussion](https://news.ycombinator.com/item?id=11918983)
- [ChucK synthesis](http://chuck.stanford.edu/doc/examples/deep/plu.ck)

```cpp
#include <stdio.h>
#include <memory.h>
#include <inttypes.h>
#include <vector>
 
// constants
const float c_pi = 3.14159265359f;
const float c_twoPi = 2.0f * c_pi;
 
// typedefs
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef int16_t     int16;
typedef int32_t     int32;
 
//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
    //the main chunk
    unsigned char m_chunkID[4];
    uint32        m_chunkSize;
    unsigned char m_format[4];
 
    //sub chunk 1 "fmt "
    unsigned char m_subChunk1ID[4];
    uint32        m_subChunk1Size;
    uint16        m_audioFormat;
    uint16        m_numChannels;
    uint32        m_sampleRate;
    uint32        m_byteRate;
    uint16        m_blockAlign;
    uint16        m_bitsPerSample;
 
    //sub chunk 2 "data"
    unsigned char m_subChunk2ID[4];
    uint32        m_subChunk2Size;
 
    //then comes the data!
};
 
//this writes
template <typename T>
bool WriteWaveFile(const char *fileName, std::vector<T> data, int16 numChannels, int32 sampleRate)
{
    int32 dataSize = data.size() * sizeof(T);
    int32 bitsPerSample = sizeof(T) * 8;
 
    //open the file if we can
    FILE *File = nullptr;
    fopen_s(&File, fileName, "w+b");
    if (!File)
        return false;
 
    SMinimalWaveFileHeader waveHeader;
 
    //fill out the main chunk
    memcpy(waveHeader.m_chunkID, "RIFF", 4);
    waveHeader.m_chunkSize = dataSize + 36;
    memcpy(waveHeader.m_format, "WAVE", 4);
 
    //fill out sub chunk 1 "fmt "
    memcpy(waveHeader.m_subChunk1ID, "fmt ", 4);
    waveHeader.m_subChunk1Size = 16;
    waveHeader.m_audioFormat = 1;
    waveHeader.m_numChannels = numChannels;
    waveHeader.m_sampleRate = sampleRate;
    waveHeader.m_byteRate = sampleRate * numChannels * bitsPerSample / 8;
    waveHeader.m_blockAlign = numChannels * bitsPerSample / 8;
    waveHeader.m_bitsPerSample = bitsPerSample;
 
    //fill out sub chunk 2 "data"
    memcpy(waveHeader.m_subChunk2ID, "data", 4);
    waveHeader.m_subChunk2Size = dataSize;
 
    //write the header
    fwrite(&waveHeader, sizeof(SMinimalWaveFileHeader), 1, File);
 
    //write the wave data itself
    fwrite(&data[0], dataSize, 1, File);
 
    //close the file and return success
    fclose(File);
    return true;
}
 
template <typename T>
void ConvertFloatSamples (const std::vector<float>& in, std::vector<T>& out)
{
    // make our out samples the right size
    out.resize(in.size());
 
    // convert in format to out format !
    for (size_t i = 0, c = in.size(); i < c; ++i)
    {
        float v = in[i];
        if (v < 0.0f)
            v *= -float(std::numeric_limits<T>::lowest());
        else
            v *= float(std::numeric_limits<T>::max());
        out[i] = T(v);
    }
}
//calculate the frequency of the specified note.
//fractional notes allowed!
float CalcFrequency(float octave, float note)
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
    return (float)(440 * pow(2.0, ((double)((octave - 4) * 12 + note)) / 12.0));
}
 
class CKarplusStrongStringPluck
{
public:
    CKarplusStrongStringPluck (float frequency, float sampleRate, float feedback)
    {
        m_buffer.resize(uint32(float(sampleRate) / frequency));
        for (size_t i = 0, c = m_buffer.size(); i < c; ++i) {
            m_buffer[i] = ((float)rand()) / ((float)RAND_MAX) * 2.0f - 1.0f;  // noise
            //m_buffer[i] = float(i) / float(c); // saw wave
        }
        m_index = 0;
        m_feedback = feedback;
    }
 
    float GenerateSample ()
    {
        // get our sample to return
        float ret = m_buffer[m_index];
 
        // low pass filter (average) some samples
        float value = (m_buffer[m_index] + m_buffer[(m_index + 1) % m_buffer.size()]) * 0.5f * m_feedback;
        m_buffer[m_index] = value;
 
        // move to the next sample
        m_index = (m_index + 1) % m_buffer.size();
 
        // return the sample from the buffer
        return ret;
    }
 
private:
    std::vector<float>  m_buffer;
    size_t              m_index;
    float               m_feedback;
};
 
void GenerateSamples (std::vector<float>& samples, int sampleRate)
{
    std::vector<CKarplusStrongStringPluck> notes;
 
    enum ESongMode {
        e_twinkleTwinkle,
        e_strum
    };
 
    int timeBegin = 0;
    ESongMode mode = e_twinkleTwinkle;
    for (int index = 0, numSamples = samples.size(); index < numSamples; ++index)
    {
        switch (mode) {
            case e_twinkleTwinkle: {
                const int c_noteTime = sampleRate / 2;
                int time = index - timeBegin;
                // if we should start a new note
                if (time % c_noteTime == 0) {
                    int note = time / c_noteTime;
                    switch (note) {
                        case 0:
                        case 1: {
                            notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 0), float(sampleRate), 0.996f));
                            break;
                        }
                        case 2:
                        case 3: {
                            notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 7), float(sampleRate), 0.996f));
                            break;
                        }
                        case 4:
                        case 5: {
                            notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 9), float(sampleRate), 0.996f));
                            break;
                        }
                        case 6: {
                            notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 7), float(sampleRate), 0.996f));
                            break;
                        }
                        case 7: {
                            mode = e_strum;
                            timeBegin = index+1;
                            break;
                        }
                    }
                }
                break;
            }
            case e_strum: {
                const int c_noteTime = sampleRate / 32;
                int time = index - timeBegin - sampleRate;
                // if we should start a new note
                if (time % c_noteTime == 0) {
                    int note = time / c_noteTime;
                    switch (note) {
                        case 0: notes.push_back(CKarplusStrongStringPluck(55.0f, float(sampleRate), 0.996f)); break;
                        case 1: notes.push_back(CKarplusStrongStringPluck(55.0f + 110.0f, float(sampleRate), 0.996f)); break;
                        case 2: notes.push_back(CKarplusStrongStringPluck(55.0f + 220.0f, float(sampleRate), 0.996f)); break;
                        case 3: notes.push_back(CKarplusStrongStringPluck(55.0f + 330.0f, float(sampleRate), 0.996f)); break;
                        case 4: mode = e_strum; timeBegin = index + 1; break;
                    }
                }
                break;
            }
        }
 
        // generate and mix our samples from our notes
        samples[index] = 0;
        for (CKarplusStrongStringPluck& note : notes)
            samples[index] += note.GenerateSample();
 
        // to keep from clipping
        samples[index] *= 0.5f;
    }
}
 
//the entry point of our application
int main(int argc, char **argv)
{
    // sound format parameters
    const int c_sampleRate = 44100;
    const int c_numSeconds = 9;
    const int c_numChannels = 1;
    const int c_numSamples = c_sampleRate * c_numChannels * c_numSeconds;
 
    // make space for our samples
    std::vector<float> samples;
    samples.resize(c_numSamples);
 
    // generate samples
    GenerateSamples(samples, c_sampleRate);
 
    // convert from float to the final format
    std::vector<int32> samplesInt;
    ConvertFloatSamples(samples, samplesInt);
 
    // write our samples to a wave file
    WriteWaveFile("out.wav", samplesInt, c_numChannels, c_sampleRate);
}
```

#### Physical audio signal processing

- [textbook](https://ccrma.stanford.edu/~jos/pasp/)
