// FM synthesis from "the synthesis of complex audio spectra by means of
// frequency modulation
#include <alsa/asoundlib.h>
#include <inttypes.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <vector>

#define PCM_DEVICE "default"

#define CLAMP(value, min, max)                                                 \
  {                                                                            \
    if (value < min) {                                                         \
      value = min;                                                             \
    } else if (value > max) {                                                  \
      value = max;                                                             \
    }                                                                          \
  }
typedef short int16;

//-32,768 to 32,767
void float2AudioSample16(float fFloat, int16 &nOut) {
  fFloat *= 32767.0f;
  CLAMP(fFloat, -32768.0f, 32767.0f);
  nOut = (int16)fFloat;
}

void NormalizeAudioData(float *pData, int nNumSamples) {
  // figure out what the maximum and minimum value is
  float fMaxValue = pData[0];
  float fMinValue = pData[0];
  for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
    if (pData[nIndex] > fMaxValue) {
      fMaxValue = pData[nIndex];
    }

    if (pData[nIndex] < fMinValue) {
      fMinValue = pData[nIndex];
    }
  }

  // calculate the center and the height
  float fCenter = (fMinValue + fMaxValue) / 2.0f;
  float fHeight = fMaxValue - fMinValue;

  // center and normalize the samples
  for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
    // center the samples
    pData[nIndex] -= fCenter;

    // normalize the samples
    pData[nIndex] /= fHeight;
  }
}

int playBufferInt16(float *audio, int numSamples, int rate, int channels) {

  float *pRawData = new float[numSamples];
  memcpy(pRawData, audio, sizeof(float) * numSamples);
  NormalizeAudioData(pRawData, numSamples);

  unsigned int pcm, tmp;
  // int rate, channels, seconds;
  snd_pcm_t *pcm_handle;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames;

  int16 *pData = new int16[numSamples];
  for (int nIndex = 0; nIndex < numSamples; ++nIndex) {
    float2AudioSample16(pRawData[nIndex], pData[nIndex]);
  }

  printf("rate: %d | channels: %d\n", rate, channels);

  /* Open the PCM device in playback mode */
  if ((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK,
                          0)) < 0)
    printf("ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE,
           snd_strerror(pcm));

  /* Allocate parameters object and fill it with default values*/
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(pcm_handle, params);

  /* Set parameters */
  // why do I need read? Oh I see, because the other way to access it is by
  // `mmap()`.
  if ((pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

  // 16 bit
  if ((pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
                                          SND_PCM_FORMAT_S16_LE) < 0))
    printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

  // two / one channel
  if ((pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0))
    printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

  // ask for rate to be close to `rate`.
  if ((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params,
                                             (unsigned int *)&rate, 0) < 0))
    printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));

  // set HW params.
  if ((pcm = snd_pcm_hw_params(pcm_handle, params) < 0))
    printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

  // print information, check that it's assigned correctly.
  printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
  printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
  snd_pcm_hw_params_get_channels(params, &tmp);
  printf("channels: %d ", tmp);

  if (tmp == 1)
    printf("(mono)\n");
  else if (tmp == 2)
    printf("(stereo)\n");

  snd_pcm_hw_params_get_rate(params, &tmp, 0);
  printf("rate: %d bps\n", tmp);

  /* Allocate buffer to hold single period */
  // Extract period size from a configuration space.
  // params	Configuration space
  // val	Returned approximate period size in frames
  // dir	Sub unit direction
  // Returns
  // 0 otherwise a negative error code if the configuration space does not
  // contain a single value Actual exact value is <,=,> the approximate one
  // following dir (-1, 0, 1)
  snd_pcm_hw_params_get_period_size(params, &frames, 0);

  int nelem_framebuf = frames * channels /* 2 -> sample size */;
  char *buff = (char *)malloc(nelem_framebuf * sizeof(int16));

  snd_pcm_hw_params_get_period_time(params, &tmp, NULL);

  int ix = 0;
  while (ix < numSamples) {
    // for (loops = (seconds * 1000000) / tmp; loops > 0; loops--) {
    // for (int i = 0; i < nelem_framebuf/sizeof(int); i++) {
    // 	memcpy(buff + i *sizeof(int), pData + ix + i, sizeof(int));
    // 	ix++;
    // }

    for (int i = 0; i < nelem_framebuf; ++i) {
      memcpy(buff + i * sizeof(int16), &pData[ix + i], sizeof(int16));
    }
    ix += nelem_framebuf;

    /*if ((pcm = fread(buff, 1, buff_size, f) == 0)) {
                    printf("Early end of file.\n");
                    return 0;
            }
    */

    if (snd_pcm_writei(pcm_handle, buff, frames) == -EPIPE) {
      printf("XRUN.\n");
      snd_pcm_prepare(pcm_handle);
    } else if (pcm < 0) {
      printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
    }
  }

  snd_pcm_drain(pcm_handle);
  snd_pcm_close(pcm_handle);
  free(buff);
  return 0;
}

// calculate the frequency of the specified note.
// fractional notes allowed!
float CalcFrequency(float octave, float note) {
  // Calculate the frequency of any note!  frequency = 440×(2^(n/12))

  // N=0 is A4
  // N=1 is A#4
  // etc...
  // notes go like so...
  // 0  = A | 1  = A# | 2  = B  | 3  = C | 4  = C# | 5  = D | 6  = D#
  // 7  = E | 8  = F  | 9  = F# | 10 = G | 11 = G#
  return (float)(440 * pow(2.0, ((double)((octave - 4) * 12 + note)) / 12.0));
}

class Drum {
public:
  Drum(float frequency, float sampleRate) :  sampleRate(sampleRate), freq(frequency) {
  }

  float GenerateSample() {
    ix++;
    const int attackDuration =  0.01*sampleRate;
    const int holdDuration = 0.01 * sampleRate;
    const int releaseDuration = 0.175 *sampleRate;

    float volume = 0;
    if (ix <= attackDuration) {
        volume = ix/(float)attackDuration;
    } else if (ix <= attackDuration + holdDuration) {
        volume = 1.0;
    }
    else if (ix <= releaseDuration + holdDuration + attackDuration) {
        float elapsed = ix - (attackDuration + holdDuration);
        float fracElapsed = std::min<float>(1, elapsed/releaseDuration);
        // freq = CalcFrequency(3, 2);
        volume = 1.0 - fracElapsed;
        // [0, 1] -> [1, 0.8]
        freq = CalcFrequency(3, 2) * (1 - 0.2*fracElapsed);
    }
    else { return 0; }

    
    // phase += CalcFrequency(3, 2); if (phase > 2*M_PI) { phase -= 2*M_PI; }
    // w = 2 pi f
    // sin(wt) = sin(f/2pi t)
    phase += 2 * M_PI * freq / sampleRate;
    return volume * sin(phase);

    // low pass filter (average) some samples
    // float value =
    //     (m_buffer[ix] + m_buffer[(ix + 1) % m_buffer.size()]) * 0.5f *
    //     m_feedback;
    // float r = (double)rand() / RAND_MAX;
    // if (r >= 1e-3) { value = -value; }
    // value = -value;

    // // float hold = 0.01; m_buffer[ix] = hold * m_buffer[ix] + (1 - hold) * value;
    // m_buffer[ix] = value;

    // // move to the next sample
    // ix = (ix + 1) % m_buffer.size();

    // // return the sample from the buffer
    // return ret;
  }

private:
  int ix;
  float phase;
  float sampleRate;
  float freq;
};

// k, begin, end;
using interpolator = float(float, float, float);

float lerp(float k, float begin, float end) {
    return (1 - k ) * begin + k * end;
}

float easeExpOut(float k, float begin, float end) {
    return lerp(k == 1 ? k : 1 - pow(2, -10 * k), begin, end);
}

float easeInOutExpo(float k, float begin, float end) {
    k =  k == 0 ? 0
        : k == 1 ? 1
        : k < 0.5 ? pow(2, 20 * k - 10) / 2
        : (2 - pow(2, -20 * k + 10)) / 2;
    return lerp(k, begin, end);
}

struct Envelope {
    float attackEndVal; 
    float attackDuration;
    float decayEndVal;
    float decayDuration;
    interpolator *decayInterpolator = &lerp;
    float sustainDuration;
    float sustainEndVal;
    float releaseDuration;
    interpolator *releaseInterpolator = &lerp;


    float val(float t) {
        if (attackDuration != 0 && t <= attackDuration) {
            return (t/attackDuration) * attackEndVal;
        };
        t -= attackDuration;
        if (decayDuration != 0 && t <= decayDuration) {
            float k = t/decayDuration;
            return decayInterpolator(k, attackEndVal, decayEndVal);
        }
        t -= decayDuration;
        if (sustainDuration != 0 && t <= sustainDuration) {
            float k = t/sustainDuration;
            return (1 - k) * decayEndVal + k*sustainEndVal;
        }
        t -= sustainDuration;
        if (t <= releaseDuration) {
            float k = t/releaseDuration;
            return releaseInterpolator(k, sustainEndVal, 0);
        }
        return 0;
    }
};

struct FM {
    float startTime; // P1
    // ? nani? why is this a separate thing from the envelope?
    float noteDuration;  // P3
    float amplitude; // P4;
    // I = d/m.
    float carrierFreq; // P5
    float modulatingFreq; // P6
    float modulationIndex1; // P7. 
    float modulationIndex2; // P8. 
    Envelope amplitudeEnvelope;
    Envelope modulationIndexEnvelope;
    float sampleRate;

    float value(float t) {
        if (t < startTime) { return 0; }
        t -= startTime; 
        t /= noteDuration;
        const float modAmp = modulationIndex1 + 
            (modulationIndex2 - modulationIndex1) * modulationIndexEnvelope.val(t/ sampleRate);
        return amplitudeEnvelope.val(t/sampleRate) *
                sin((2 * M_PI * carrierFreq* t)/sampleRate + modAmp * sin((2 * M_PI * modulatingFreq * t) / sampleRate));
    }
};

FM mkBrass(float freq, float startTime, float duration, float samplerate) {
  FM fm;
  fm.startTime = startTime;
  fm.noteDuration = duration;
  fm.carrierFreq = freq;
  fm.modulatingFreq = fm.carrierFreq;
  fm.modulationIndex1 = 0;
  fm.modulationIndex2 = 5;

  fm.sampleRate = samplerate;

  fm.amplitudeEnvelope.attackDuration = 0.3;
  fm.amplitudeEnvelope.attackEndVal = 1;
  fm.amplitudeEnvelope.decayDuration = 0.3; // 2/6 = 1/3
  fm.amplitudeEnvelope.decayEndVal = 0.75;
  fm.amplitudeEnvelope.sustainDuration = 0.6;
  fm.amplitudeEnvelope.sustainEndVal = 0.6;
  fm.amplitudeEnvelope.releaseDuration = 0.3;
  fm.modulationIndexEnvelope = fm.amplitudeEnvelope;
  return fm;
}

FM mkReed(float freq, float startTime, float duration, float samplerate) {
  FM fm;
  fm.startTime = startTime;
  fm.noteDuration = duration;
  fm.carrierFreq = freq;
  // 900 carrier -> 600  modulator
  fm.modulatingFreq = freq*2/3;
  fm.modulationIndex1 = 4;
  fm.modulationIndex2 = 2;

  fm.sampleRate = samplerate;

  fm.amplitudeEnvelope.attackDuration = 0.1;
  fm.amplitudeEnvelope.attackEndVal = 1;
  fm.amplitudeEnvelope.decayDuration = 0; // 2/6 = 1/3
  fm.amplitudeEnvelope.decayEndVal = 1;
  fm.amplitudeEnvelope.sustainDuration = 0.4;
  fm.amplitudeEnvelope.sustainEndVal = 1;
  fm.amplitudeEnvelope.releaseDuration = 0.05;

  fm.modulationIndexEnvelope.attackDuration = 0.1;
  fm.modulationIndexEnvelope.attackEndVal = 1;
  fm.modulationIndexEnvelope.decayDuration = 0;
  fm.modulationIndexEnvelope.decayEndVal = 1;
  fm.modulationIndexEnvelope.sustainDuration = 0.6; // hold for full life of woodwind.
  fm.modulationIndexEnvelope.sustainEndVal = 1;
  fm.modulationIndexEnvelope.releaseDuration = 0.05;
  return fm;
}

FM mkBell(float freq, float startTime, float duration, float samplerate) {
  FM fm;
  fm.startTime = startTime;
  fm.noteDuration = 1;
  fm.carrierFreq = freq;
  // 200 / 280 = 
  fm.modulatingFreq = freq * 1.4;
  fm.modulationIndex1 = 0;
  fm.modulationIndex2 = 4;

  fm.sampleRate = samplerate;

  fm.amplitudeEnvelope.attackDuration = 0;
  fm.amplitudeEnvelope.attackEndVal = 1;
  fm.amplitudeEnvelope.decayDuration = 0;
  fm.amplitudeEnvelope.decayEndVal = 1;
  fm.amplitudeEnvelope.sustainDuration = 0;
  fm.amplitudeEnvelope.sustainEndVal = 1;
  fm.amplitudeEnvelope.releaseDuration = 15;
  fm.amplitudeEnvelope.releaseInterpolator = easeExpOut;

  fm.modulationIndexEnvelope = fm.amplitudeEnvelope;
  return fm;
}

FM mkDrum(float freq, float startTime, float duration, float samplerate) {
  FM fm;
  fm.startTime = startTime;
  fm.noteDuration = 1;
  fm.carrierFreq = freq;
  // 200 / 280 = 
  fm.modulatingFreq = freq * 1.4;
  fm.modulationIndex1 = 0;
  fm.modulationIndex2 = 2;

  fm.sampleRate = samplerate;

  fm.amplitudeEnvelope.attackDuration = 0;
  fm.amplitudeEnvelope.attackEndVal = 0.8;
  fm.amplitudeEnvelope.decayDuration = 0.05;
  fm.amplitudeEnvelope.decayEndVal = 1;
  fm.amplitudeEnvelope.decayInterpolator = easeInOutExpo;
  fm.amplitudeEnvelope.sustainDuration = 0;
  fm.amplitudeEnvelope.sustainEndVal = 1;
  fm.amplitudeEnvelope.releaseDuration = 0.9;
  fm.amplitudeEnvelope.releaseInterpolator = easeExpOut;

  fm.modulationIndexEnvelope.attackDuration = 0;
  fm.modulationIndexEnvelope.attackEndVal = 1;
  fm.modulationIndexEnvelope.decayDuration = 0;
  fm.modulationIndexEnvelope.decayEndVal = 1;
  fm.modulationIndexEnvelope.sustainDuration = 0;
  fm.modulationIndexEnvelope.sustainEndVal = 1;
  fm.modulationIndexEnvelope.releaseDuration = 15;
  fm.modulationIndexEnvelope.releaseInterpolator = easeExpOut;
  return fm;
}

FM mkWoodDrum(float freq, float startTime, float duration, float samplerate) {
  FM fm;
  fm.startTime = startTime;
  fm.noteDuration = 1;
  fm.carrierFreq = freq;
  // 200 / 280 = 
  fm.modulatingFreq = freq * 55.0 / 80.0;
  fm.modulationIndex1 = 0;
  fm.modulationIndex2 = 25;

  fm.sampleRate = samplerate;

  fm.amplitudeEnvelope.attackDuration = 0;
  fm.amplitudeEnvelope.attackEndVal = 0.9;
  fm.amplitudeEnvelope.decayDuration = 0.05;
  fm.amplitudeEnvelope.decayEndVal = 1;
  fm.amplitudeEnvelope.decayInterpolator = easeInOutExpo;
  fm.amplitudeEnvelope.sustainDuration = 0;
  fm.amplitudeEnvelope.sustainEndVal = 1;
  fm.amplitudeEnvelope.releaseDuration = 0.9;
  fm.amplitudeEnvelope.releaseInterpolator = easeExpOut;


  fm.modulationIndexEnvelope.attackDuration = 0;
  fm.modulationIndexEnvelope.attackEndVal = 1;
  fm.modulationIndexEnvelope.decayDuration = 0;
  fm.modulationIndexEnvelope.decayEndVal = 1;
  fm.modulationIndexEnvelope.sustainDuration = 0;
  fm.modulationIndexEnvelope.sustainEndVal = 1;
  fm.modulationIndexEnvelope.releaseDuration = 0.02;
  return fm;
}


int main() {
  // sound format parameters
  const int c_sampleRate = 44100;
  const int c_numSeconds = 6;
  const int c_numChannels = 1;

  // make space for our samples
  std::vector<float> samples;

  std::vector<FM> fms;
  // fms.push_back(mkBrass(CalcFrequency(3, 1), /*start=*/0, /*duration=*/2, c_sampleRate));
  // fms.push_back(mkBell(CalcFrequency(4, 1), /*start=*/0, /*duration=*/2, c_sampleRate));
  fms.push_back(mkDrum(CalcFrequency(2, 1), /*start=*/0, /*duration=*/2, c_sampleRate));
  // fms.push_back(mkWoodDrum(CalcFrequency(2, 1), /*start=*/0, /*duration=*/2, c_sampleRate));
  // fms.push_back(mkReed(CalcFrequency(3, 1), /*start=*/c_sampleRate * 2, /*duration=*/2, c_sampleRate));
  // fms.push_back(mkReed(CalcFrequency(2, 8), /*start=*/c_sampleRate, /*duration=*/0.5, c_sampleRate));
  // fms.push_back(mkReed(CalcFrequency(2, 8), /*start=*/c_sampleRate, /*duration=*/0.5, c_sampleRate));
  // fms.push_back(mkReed(CalcFrequency(3, 1), /*start=*/c_sampleRate, /*duration=*/2, c_sampleRate));
  // fms.push_back(mkReed(CalcFrequency(3, 1), /*start=*/2*c_sampleRate, /*duration=*/1, c_sampleRate));

  int c_numSamples = c_sampleRate * 20;
  for (int i = 0; i < c_numSamples; ++i) {
      float out = 0;
      for(FM fm : fms) { out += fm.value(i); };
      samples.push_back(out);
  }

  // for (int i = 0; i < 0.5*c_numSamples; ++i) {
  //   float out = 0;
  //   for(int j = 0; j < NNOTES; ++j) {
  //       out += notes[j].GenerateSample();
  //   }
  //   samples.push_back(out);
  // }

  // generate samples
  // GenerateSamples(samples, c_sampleRate);
  playBufferInt16(samples.data(), samples.size(), c_sampleRate, c_numChannels);

  return 0;
}
