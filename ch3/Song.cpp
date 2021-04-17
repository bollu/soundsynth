/*===================================================

Song.cpp

Chapter 3 song

Written by Alan Wolfe 6/2012

===================================================*/

#include "Song.h"
#include "AudioEventMgr.h"
#include "AudioEvent_Melody1.h"
#include "AudioEvent_Melody2.h"
#include "AudioEvent_Melody3.h"
#include "AudioEvent_PlayWav.h"
#include "stdio.h"
#include "AudioEvent_PlayWavRepeat.h"


int play(const char *filename, int rate, int channels, int seconds);
void RenderSong(void) {
  // define our audio event manager with our audio settings
  const int samplerate = 44100;
  const int nchannels = 2;
  const int nseconds = 130;
  CAudioEventMgr AudioEventMgr(samplerate, nchannels, nseconds);

  // the intro sound clip: 0 to 17 seconds, fading out over the last 2 seconds
  CAudioEvent_PlayWav *pPlayWav =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWav>(0.0f, 17.0f);
  pPlayWav->Init("SourceWaves/legend1.wav", 3.0f, 0.0f, 1.0f, 2.0f);

  // intro drum part from 15 to 19 seconds
  CAudioEvent_PlayWavRepeat *pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(15.0f, 19.0f);
  pPlayWavRepeat->Init("SourceWaves/ting.wav", 1.3f, 0.5f);

  // the main drum beat from 19 seconds on
  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(19.0f, 123.0f);
  pPlayWavRepeat->Init("SourceWaves/kick.wav", 1.3f, 1.0f);

  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(19.25f, 123.0f);
  pPlayWavRepeat->Init("SourceWaves/ting.wav", 1.3f, 0.5f);

  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(19.5f, 41.0f);
  pPlayWavRepeat->Init("SourceWaves/clap.wav", 1.3f, 1.0f);

  // the base melody starting in at 25 seconds
  CAudioEvent_Melody1 *pMelody1 =
      AudioEventMgr.AddEvent<CAudioEvent_Melody1>(23.0f, 123.0f);

  // fast hand claps
  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(41.0f, 43.0f);
  pPlayWavRepeat->Init("SourceWaves/clap.wav", 2.0f, 0.25f);

  // back to regular hand claps
  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(43.5f, 123.0f);
  pPlayWavRepeat->Init("SourceWaves/clap.wav", 1.3f, 1.0f);

  // the regular melody starts at 43 seconds
  CAudioEvent_Melody2 *pMelody2 =
      AudioEventMgr.AddEvent<CAudioEvent_Melody2>(43.0f, 123.0f);

  // the top melody starts at 59 seconds
  CAudioEvent_Melody3 *pMelody3 =
      AudioEventMgr.AddEvent<CAudioEvent_Melody3>(59.0f, 123.0f);

  // a sound to end the music portion
  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(123.0f, 124.0f);
  pPlayWavRepeat->Init("SourceWaves/clap.wav", 1.0f, 1.0f);
  pPlayWavRepeat =
      AudioEventMgr.AddEvent<CAudioEvent_PlayWavRepeat>(123.0f, 124.0f);
  pPlayWavRepeat->Init("SourceWaves/kick.wav", 1.3f, 1.0f);

  // play the closing quote at 125
  pPlayWav = AudioEventMgr.AddEvent<CAudioEvent_PlayWav>(123.0f, 130.0f);
  pPlayWav->Init("SourceWaves/legend2.wav", 3.0f, 0.0f, 1.0f, 2.0f);

  // render the wav file
//   AudioEventMgr.RenderWaveFileInt16("LamentOfTimCurry.wav");
//   play("LamentOfTimCurry.wav", samplerate, nchannels, nseconds);
  printf("trying to play file:\n");
  AudioEventMgr.playWaveFileInt16();
}
