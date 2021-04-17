/*===================================================

Written by Alan Wolfe 5/2012

http://demofox.org

===================================================*/

/*
 * Simple sound playback using ALSA API and libasound.
 *
 * Compile:
 * $ cc -o play sound_playback.c -lasound
 * 
 * Usage:
 * $ ./play <sample_rate> <channels> <seconds> < <file>
 * 
 * Examples:
 * $ ./play 44100 2 5 < /dev/urandom
 * $ ./play 22050 1 8 < /path/to/file.wav
 *
 * Copyright (C) 2009 Alessandro Ghedini <alessandro@ghedini.me>
 * --------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Alessandro Ghedini wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can
 * buy me a beer in return.
 * --------------------------------------------------------------
 */

#define PCM_DEVICE "default"


#include <alsa/asoundlib.h>
#include <stdio.h>

int play(const char *filename, int rate, int channels, int seconds) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) { assert(false && "unable to open wav file"); }
	unsigned int pcm, tmp, dir;
	// int rate, channels, seconds;
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	char *buff;
	int buff_size, loops;

	// if (argc < 4) {
	// 	printf("Usage: %s <sample_rate> <channels> <seconds>\n",
	// 							argv[0]);
	// 	return -1;
	// }

	// rate 	 = atoi(argv[1]);
	// channels = atoi(argv[2]);
	// seconds  = atoi(argv[3]);

	/* Open the PCM device in playback mode */
	if ((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
					SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
		printf("ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set parameters */
	if ((pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

	if ((pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
						SND_PCM_FORMAT_S16_LE) < 0))
		printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

	if ((pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0)) 
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

	if ((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, (unsigned int *)&rate, 0) < 0))
		printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));

	/* Write parameters */
	if ((pcm = snd_pcm_hw_params(pcm_handle, params) < 0))
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

	/* Resume information */
	printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
	printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
	snd_pcm_hw_params_get_channels(params, &tmp);
	printf("channels: %i ", tmp);

	if (tmp == 1)
		printf("(mono)\n");
	else if (tmp == 2)
		printf("(stereo)\n");

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	printf("rate: %d bps\n", tmp);

	printf("seconds: %d\n", seconds);	

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(params, &frames, 0);

	buff_size = frames * channels * 2 /* 2 -> sample size */;
	buff = (char *) malloc(buff_size);

	snd_pcm_hw_params_get_period_time(params, &tmp, NULL);

	for (loops = (seconds * 1000000) / tmp; loops > 0; loops--) {

		if ((pcm = fread(buff, 1, buff_size, f) == 0)) {
			printf("Early end of file.\n");
			return 0;
		}

		if ((pcm = snd_pcm_writei(pcm_handle, buff, frames) == -EPIPE)) {
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



#include "Song.h"
#include "oscillators.h"
#include "utils.h"
#include "wavefile.h"

// this is for the bandlimited noise test
#include <stdlib.h>

// the entry point of our application
int main(int argc, char **argv) {
    // our parameters that all the wave forms use
    int nSampleRate = 44100;
    int nNumSeconds = 4;
    int nNumChannels = 1;
    float fFrequency = CalcFrequency(3, 3);  // middle C

    // make our buffer to hold the samples
    int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
    float *pData = new float[nNumSamples];

    // the phase of our oscilator
    float fPhase = 0;

    // make a band limited saw wave
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] = AdvanceOscilator_Saw_BandLimited(fPhase, fFrequency,
                                                         (float)nSampleRate);
    }

    WriteWaveFileInt16("SawBL.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make a spacey noise
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] = AdvanceOscilator_Saw_BandLimited(
            fPhase, fFrequency * ((float)nIndex / (float)nNumSamples),
            (float)nSampleRate);
    }

    WriteWaveFileInt16("Spacey.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make an aliasing saw wave
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] =
            AdvanceOscilator_Saw(fPhase, fFrequency, (float)nSampleRate);
    }

    WriteWaveFileInt16("Saw.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make a band limited square wave
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] = AdvanceOscilator_Square_BandLimited(fPhase, fFrequency,
                                                            (float)nSampleRate);
    }
    WriteWaveFileInt16("SquareBL.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make an aliasing square wave
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] =
            AdvanceOscilator_Square(fPhase, fFrequency, (float)nSampleRate);
    }
    WriteWaveFileInt16("Square.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make a band limited triangle wave
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] = AdvanceOscilator_Triangle_BandLimited(
            fPhase, fFrequency, (float)nSampleRate, 2);
    }
    WriteWaveFileInt16("TriangleBL.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make an aliasing triangle wave
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] =
            AdvanceOscilator_Triangle(fPhase, fFrequency, (float)nSampleRate);
    }
    WriteWaveFileInt16("Triangle.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make some band limited random beeps
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        if (nIndex % 4000 == 0) {
            fFrequency = CalcFrequency(0, (float)(rand() % 115));
        }
        pData[nIndex] =
            AdvanceOscilator_Sine(fPhase, fFrequency, (float)nSampleRate);
    }
    WriteWaveFileInt16("RandomBeeps.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // try making some bandlimited noise by making the band limited beeps faster
    fPhase = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        if (nIndex % 40 == 0) {
            fFrequency = CalcFrequency(0, (float)(rand() % 115));
        }
        pData[nIndex] =
            AdvanceOscilator_Sine(fPhase, fFrequency, (float)nSampleRate);
    }
    WriteWaveFileInt16("NoiseBL.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // make some aliasing noise
    fPhase = 0;
    fFrequency = CalcFrequency(3, 3);
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        pData[nIndex] =
            AdvanceOscilator_Noise(fPhase, fFrequency, (float)nSampleRate,
                                   nIndex > 0 ? pData[nIndex - 1] : 0.0f);
    }
    WriteWaveFileInt16("Noise.wav", pData, nNumSamples, nNumChannels,
                       nSampleRate);

    // render an example song
    RenderSong();

    // free our data buffer
    delete[] pData;
}
