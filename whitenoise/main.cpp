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

    if ((pcm = snd_pcm_writei(pcm_handle, buff, frames)) == -EPIPE) {
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

class CKarplusStrongStringPluck {
public:
  CKarplusStrongStringPluck(float frequency, float sampleRate, float feedback) {
    m_buffer.resize((float(sampleRate) / frequency));
    for (size_t i = 0, c = m_buffer.size(); i < c; ++i) {
      m_buffer[i] = ((float)rand()) / ((float)RAND_MAX) * 2.0f - 1.0f; // noise
    }
    ix = 0;
    m_feedback = feedback;
  }

  float GenerateSample() {
    // get our sample to return
    float ret = m_buffer[ix];

    // low pass filter (average) some samples
    float value =
        (m_buffer[ix] + m_buffer[(ix + 1) % m_buffer.size()]) * 0.5f *
        m_feedback;
    m_buffer[ix] = value;

    // move to the next sample
    ix = (ix + 1) % m_buffer.size();

    // return the sample from the buffer
    return ret;
  }

private:
  std::vector<float> m_buffer;
  size_t ix;
  float m_feedback;
};

int main() {
  // sound format parameters
  const int c_sampleRate = 44100;
  const int c_numSeconds = 6;
  const int c_numChannels = 1;
  const int c_numSamples = c_sampleRate * c_numChannels * c_numSeconds;

  // make space for our samples
  std::vector<float> samples;
  samples.resize(c_numSamples);
  


  for (int i = 0; i < c_numSamples; ++i) {
    samples[i] = rand() % RAND_MAX;
    }

  // generate samples
  // GenerateSamples(samples, c_sampleRate);
  playBufferInt16(samples.data(), c_numSamples, c_sampleRate, c_numChannels);

  return 0;
}
