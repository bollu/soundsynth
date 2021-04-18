# Kavariance

`bollu` learns digital audio synthesis from [the blog at the bottom of the sea](https://blog.demofox.org/2012/05/14/diy-synthesizer-chapter-1-sound-output/).
the name is a pun on [Kaveri](https://en.wikipedia.org/wiki/Kaveri), A `wav`/river goddess,
and [Covariance](https://en.wikipedia.org/wiki/Covariance)

Sources are:
1. BasicSynth
1. [textbook](https://ccrma.stanford.edu/~jos/pasp/)
1. [demofox / blog under the ocean audio synth tutorials](http://demofox.org)

# Notes: Ch1

- sound: ?
- Frequency (what musicians call pitch)
- time interval between samples: **sample time** (note, this is a DELTA quantity!)
- samples per second : frequency / *sample rate*.
- sample rate determines what frequencies we can produce!
- To represent a perodic signal, we need at min, one value above,
  one value below zero. So we need at least two samples per cycle.
  Thus, if I can produce *samples* at a rate of `fs`, I can produce
  a periodic sinusoidal waveform at a rate of `fmax = fs/2`, to have values
  above and below zero. This is the Nyquist shannon theorem.
- Upper limit for sound: 20,000 Hz. So we use sampling frequency > 40,000 Hz.
- Standard: 44,100 Hz.
- Number of samples generated for a given *sound*: duration x sample rate.
- Sample number: time in seconds x number of samples per second; `S = T fs`.
  Intuitively, discretize time in sample space.
- For sample number bit-size, we estimate that at a rate of 44,100 Hz, a 32 bit
  number can represent `(1 << 31)/44100` sample numbers ~= 13.5 hours. Plenty
  for any song making.
- For amplitude bit-size, we want to take into account dynamic range of hearing.
  We can't detect much difference across the range of hearing.
  So we use decibels, which are logarithmic. 1db = smallest change in amplitude
  humans can hear. `AdB = 20 log_10 (A1/A0)`. Each new bit we use will give us
  `20 log_10(2/1) ~= 6 dB`. 16 bit value can hold `16x6 = 96 dB`. Music has a dynamic
  range of around `100 dB`, so 16-bit works well for us.
>  The range of hearing is quite
> large, a ratio of about 10^12 to 1.  [Sid: I don't understand what this means].


# Notes: Ch2
- PhsAccum: phase
- FrqValue: frequency
- AmpValue: amplitude
- SampleRate: `44100`.
- SampleScale: convert AmpValue to output range: `(1<<15) - 1 = 32767`.

# Ch4

- `WAVE` file  is a `RIFF` file (resource interchange file format).
- nested chunks of data.
- chunk header:`8` byte [ charID:`4`byte / 4 chars; chunksize:`4` byte / integer-little-endian]
- We consider `WAVE` to be a fixed length header, followed by variable length data.
  (1) Write the header. (2) Write the samples. Each sample is one 16 bit word per channel.
- For stereo output, two samples are written interleaved: [left channel; right channel]

# Ch5: signal generator
- To generate a signal we need to know (1) number of samples in one period and
  (2) phase increment for one sample time / phase velocity `φ`.
- Solving for (1): Recall that Period/`T`: time taken for waveform to repeat. Reciprocal of frequency; `T=1/f`.
  Multiplying sample rate `fs` with time of one period `1/f` gives us number of samples
  per period `fs/f`.
- Solving for (2): The total phase is `2pi`. So the phase increment per sample time / phase velocity
  is `φ = 2pi/(fs/f)`.
- The value of the nth sample `s[n]` is equal to the peak volume at sample `n`
  `An` (ie, the radius of the circle at that sample), times the sin of the phase angle at
  sample n, `θ[n]`. `s[n] = A[n] sin(θ[n])`. The nth phase angle `θ[n]` is the phase
  increment per sample / phase velocity times the number of samples:  `θ[n] = φ * n`

- In total, this gives us `s[n] = A[n] θ[n] = A[n] sin(φ * n) = A[n] sin (2π/fs * f * n)`.
  Implemented as:

```cpp
frqRad = twoPI / sampleRate;
totalSamples = duration * sampleRate;
for (n = 0; n < totalSamples; n++) {
    sample[n] = volume * sin(frqRad*frequency*n);
}
```

This can be optimised into:

```cpp
frqRad = twoPI / sampleRate;
phaseIncr = frqRad * frequency;
phase = 0; volume = 1;
totalSamples = duration * sampleRate;
for (n = 0; n < totalSamples; n++) {
  sample[n] = volume * sin(phase);
  phase += phaseIncr; // keep track of phase
  if (phase >= twoPI) { phase -= twoPI; }
}
```

- The duration value is specified in seconds and must be multiplied by the
  sample rate to determine the number of samples.
- Musicians express frequency (Hz) as a pitch or possibly as a number representing
  a key on a keyboard.  Frequency can be calculated directly from the pitch or
  key number. Each half-step between notes in the musical scale represents a
  multiplication by the frequency ratio of `2^(1/12) (~1.059)` (a) 12 because
  there are 12 notes: `A...G`, (b) base `2` because logarithm?. Given a known
  pitch of frequency `f0` and the number of half-steps between pitches `h`, the
  frequency of any pitch is (`f0·2^(h/12)`). 
- Pitch is thus an *index* into a frequency table.
- [`Include/SynthDefs.h`](https://github.com/bollu/basicsynth/blob/master/Include/SynthDefs.h)
- [`Include/GenWave.h`](https://github.com/bollu/basicsynth/blob/master/Include/GenWave.h)
- [`Src/Common/Global.cpp`](https://github.com/bollu/basicsynth/blob/master/Src/Common/Global.cpp)

# Ch6: Envelope generators

- ASR: attack sustain release. "release" is from analog synths where the envelope generator
  was triggered when a key on the keeb was pressed.

```cpp
phaseIncr = frqRad * frequency;
phase = 0;
totalSamples = duration * sampleRate;
attackTime = attackRate * sampleRate;
decayTime = decayRate * sampleRate;
decayStart = totalSamples – decayTime;
if (attackTime > 0) {
    envInc = peakAmp / attackTime; // derivative
}
volume = 0;
for (n = 0; n < totalSamples; n++) {
    if (n < attackTime || n > decayStart) {
        volume += envInc;
    } else if (n == attackTime) {
         // get rid of roundoff err
        volume = peakAmp;
    } else if (n == decayStart) {
        envInc = –volume / decayTime;
    }
    sample[n] = volume * sin(phase);
    if ((phase += phaseIncr) >= twoPI)
        phase -= twoPI;
}
```
 
- A linear change in amplitude does NOT produce a linear change in perceived
  loudness.
- In order to double the loudness of a sound, we need to double the power of
  the sound, and that requires an exponential increase in amplitude(!)

- ADSR envelope: attack, decay, sustain, release.

# Ch7: Complex waveforms

- In the case of a square wave there are little “horns” where the transition
  from peak to peak overshoots a little and then settles back. These effects
  are the result of summing a finite series of partials rather than the
  infinite series of partials specified by the Fourier transform. The ripple
  and overshoot is called the **Gibbs Phenomenon** after the engineer who
  identified the cause.
- Eliminate the ripple by (1) using a large number of partials, and (2)
  multiply the amplitude of all partials above the fundamental with a small
  value called the **Lanczos sigma**: `σ = sin(x)/x`, where `x = nπ/M`, where `M`
  is the total number of partials and `n` is the partial number.
- FM paper: `The Synthesis of Complex Audio Spectra by Means of Frequency Modulation`.
- in FM, one oscialltor varies another oscillator: `f(t) = Ac sin(ωc t + (Am sin(ωm t ))`
  where `c` is for carrier, `m` is for modulator.
- Modulator frequency is usually specified as a multiple of the carrier and
  identified by the term c:m ratio. In general, integer multiples produce a
  harmonic spectrum while non-integer multiples produce an inharmonic spectrum.
- Fixed modulator amplitude affects different carriers differently. Eg, if
  modulator amplitude is `50Hz`, and carrier is `100Hz`, then we get a band of
  `50-150Hz`, ie, `50%` delta from the "center". On the other hand, if carrier
  is a `1000Hz`, then we get a band of `950-1050Hz`, ie, `5%` delta from the
  "center".
- So, good way to talk about influence of modulator on carrier is in terms
  of the **index of modulation**: `I = Δf/fm` where `Δf` is the maximum frequency
  deviation, and `fm` is the modulator frequency.
- Amplitude modulation: change amplitude by a sine wave.
- White noise: noise of all frequencies. Just generate samples using a PRNG.
- Low rumbly noise: generate noise, but hold the same sample for multiple sample times.
  This makes it seem rumbly (!). [bollu: alternatively, run a low-pass filter].

# Ch8: Wavetable oscillators
- Store table, linearly intepolate values that are not keyed (fractional indexes).
- Use fixed-point for table.

# Ch9: Mixing and panning
- Mixer: literally keep an out variable for channels (one for mono, two for stereo)
  and add in all contributions.
- Panning: creating an apparent angle of displacement from a position directly in front
  of the listener. We can pan by setting volume of left and right. This will create
  a sense of spatiality of the sound. Usually, we do this by having a "pan value"
  from the left, and scaling the left volume by `panLeft` and the right volume by `(1 - panLeft)`.
- The linear calculation of the pan amplitude is simple and efficient but
  produces the well-known “hole in the middle effect" (!) (TODO), where the
  change in intensity causes us to perceive that the sound has moved
  away/closer instead of left/right.
- One way to perform a better calculation is to take a `sin` of apparent angle. The pan angle
  can change from `-45` to `+45` degrees, with `0` being centered. We normalize the
  angle to `[-1, +1]`. The amplitudes are calculated as:

```cpp
leftPan = sin((1 - panSet)/2 * PI/2);
rightPan = sin((1 + panSet)/2 * PI/2);
```

# Ch10: Digital filters

- Delay is represented by `z^(-1)`.
- Adding up delayed sinusoids creates a phase shift in the output! `sin(a) + sin(b) = 2sin((a+b)/2)sin((a-b)/2)`.
- Phase shift depends on sample rate and frequency of signal. If sample rate is `10kHz`,
  a one sample delay is `0.0001` seconds of delay. For a signal of 100Hz, the time period
  is `1/100 = 0.01` seconds. So, a delay of `0.0001` seconds in a total time
  period of `0.01` seconds represents `2pi * 0.01/0.0001 = 3.6 degree` of phase
  shift.
- A signal at `1000Hz` has a time period of `1/1000 = 0.001` seconds. So a delay of `0.0001`
  seconds in a total time period of `0.001` seconds represents a `2pi * 0.001 / 0.0001 = 36 degree`
  phase shift. This phase shift causes a lower amplitude, and  causes destructive interference at higher
  frequencies. So this implements a low-pass filter. For a high pass filter, we subtract the signal
  with its previous timestep.

- FIR: finite impulse response filter: `y[n] = ax[n] + bx[n-1]`.
- IIR: infinite impulse response: `y[n] = bx[n] + ay[n-1]`.
- Bi-quad filter: `y[n] = b0(x[n] - a1 y[n-1] - a2 y[n-2]) + b1 x[n-1] + b2 x[n-1]`.
  That is, it has two degrees of "feedback" and two degrees of "history".
- For low-pass FIR, calculate impulse response by convolving with windowed `sinc`.
- Window a `sinc` by multiplying with a ["hamming window"/"hanning window"](https://en.wikipedia.org/wiki/Hann_function)
- Bi-quad filter can be adapted for many tasks. For example, butterworth filter.
- All-pass filter: filter that allows all frequencies, but shifts phase in a *frequency-dependent fashion*.
  `v[n] = x[n] - g v[n-1]` and `y[n] = g v[n] + v[n-1]`.
- Dynamic filter:

# Ch11: Delay lines

- Delay ~ FIFO.
- Accoustics: delay line ~ echo.
- attenuator after delay ~ loss of signal energy in the echo.
- `total time delay = length of delay line * sample time.`
- can tap from the middle of a delay line as well.
- use ring buffer.

- Resonator: allow signal in delay line to recirculate

```cpp
out = delayBuf[delayIndex] * decay;
delayBuf[delayIndex] = in + out;
if (++delayIndex >= delayLen)
delayIndex = 0;
```
- Allpass delay: TODO (???)
- Variable delay: vary delay time while adding and removing samples. We keep the length
 of the delay buffer constant, but we move the read tap around.

```cpp
delayRange = (delayMax - delayMin) / 2;
delayMid = delayMin + delayRange;
delayBuf[delayWrite] = in; // write data in
if (++delayWrite >= delayLen) { delayWrite -= delayLen; } // go back 
delayOffset = delayMid + (delayRange * sin(phase));
if ((delayRead = delayWrite – delayOffset) < 0) { delayRead += delayLen; }
out = delayBuf[delayRead];
```

- Problem: index will move by fractional amout. This creates an artefact called
  [**zipper noise**](https://www.sweetwater.com/insync/zipper-noise)
  which is caused by quantization. So we need to LERP.

```cpp
delayOffset = delayMid + (delayRange * sin(phase));
delayRead = delayWrite – delayOffset;
if (delayRead < 0)
delayRead += delayLen;
readInt = floor(delayRead);
readFract = delayRead – readInt;
if (delayOffset < 1)
delayBuf[delayWrite] = in;
out = delayBuf[readInt] * (1 – readFract);
if (--readInt < 0)
readInt += delayLen;
out += delayBuf[readInt] * readFract;
if (delayOffset >= 1)
delayBuf[delayWrite] = in;
if (++delayWrite >= delayLen)
delayWrite -= delayLen;
out *= decay;
```




# DemoFox tutorials

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

# TODO
- rainmood sounds! So, lightning crack, rumbling.
- fireplace.
- coffee shop clinking?
