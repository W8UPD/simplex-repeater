/*
  W8UPD Simplex Repeater - A simplex repeater
  Copyright (C) 2012 W8UPD

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <pthread.h>
#include <unistd.h>
#include "tonedecode.h"

#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (60)
#define NUM_CHANNELS    (1)
#define DITHER_FLAG     (0) 

#define WRITE_TO_FILE   (0)

#define PA_SAMPLE_TYPE  paInt16
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"

char rcvd;
extern float toneAvg;
extern float sum;

static int record( const void *inBuf, void *outBuf,
		   unsigned long framesPerBuffer,
		   const PaStreamCallbackTimeInfo* timeInfo,
		   PaStreamCallbackFlags statusFlags,
		   void *userData )
{
  paData *data = (paData*)userData;
  const short *prwrite = (const short*)inBuf;
  short *pwrite = &data->recAud[data->frameIndex * NUM_CHANNELS];
  long framesToCalc;
  long i;
  int finished;
  unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

  (void) outBuf;
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  if (framesLeft < framesPerBuffer) {
      framesToCalc = framesLeft;
      finished = paComplete;
  } else {
      framesToCalc = framesPerBuffer;
      finished = paContinue;
  }

  if (inBuf == NULL) {

    for(i=0; i<framesToCalc; i++) {
      *pwrite++ = SAMPLE_SILENCE;
    }

  } else {

    for(i=0; i<framesToCalc; i++) {
      *pwrite++ = *prwrite++;
    }

  }

  data->frameIndex += framesToCalc;

  //printf(":%c %f-", dtmf(data->recAud), toneAvg);
  dtmf(data->recAud);

  return finished;
}

static int play( const void *inBuf, void *outBuf,
		 unsigned long framesPerBuffer,
		 const PaStreamCallbackTimeInfo* timeInfo,
		 PaStreamCallbackFlags statusFlags,
		 void *userData )
{
  paData *data = (paData*)userData;
  short *prwrite = &data->recAud[data->frameIndex * NUM_CHANNELS];
  short *pwrite = (short*)outBuf;
  unsigned int i;
  int finished;
  unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

  (void) inBuf;
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  if(framesLeft < framesPerBuffer) {
    //last buffer read
    for (i=0; i<framesLeft; i++) {
      *pwrite++ = *prwrite++;
    }

    for (; i<framesPerBuffer; i++) {
	  *pwrite++ = 0;
    }

    data->frameIndex += framesLeft;
    finished = paComplete;

  } else {

    for (i=0; i<framesPerBuffer; i++) {
	  *pwrite++ = *prwrite++;
    }

    data->frameIndex += framesPerBuffer;
    finished = paContinue;

  }

  dtmf(data->recAud);
  return finished;
}

int oh_no_what_now(paData data, int err)
{
  Pa_Terminate();
  if (data.recAud)
    free(data.recAud);
  if (err != paNoError) {
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    err = 1;
  }
  return err;
}

int sound() {
  //some starting stuff
  PaStreamParameters inputParameters, outputParameters;
  PaStream* stream;
  PaError err = paNoError;
  paData data;
  int i;
  int totalFrames;
  int numSamples;
  int numBytes;
  short val;
  int TX = 0;
  int RX = 1;
  int thresh = 15000;
  int endat = 0;
  int slope = 2;


  //more starting stuff
  data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; //Record for...
  data.frameIndex = 0; //zero position
  numSamples = totalFrames * NUM_CHANNELS;
  numBytes = numSamples * sizeof(short);

  //allocate memory for recorded audio
  data.recAud = (short *) malloc( numBytes ); /* recAud is initialised. */
  if (data.recAud == NULL) {
      printf("Could not allocate record array.\n");
      return oh_no_what_now(data, err);
  }

  //clear data.recAud array
  for (i=0; i<numSamples; i++) data.recAud[i] = 0;

  //initialize portAudio
  err = Pa_Initialize();
  if (err != paNoError)
    return oh_no_what_now(data, err);

  //use the default device
  inputParameters.device = Pa_GetDefaultInputDevice(); 
  if (inputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default input device.\n");
    return oh_no_what_now(data, err);
  }

  //tell portaudio the parameters for the device
  inputParameters.channelCount = NUM_CHANNELS;
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  //open stream for recording and playback
  err = Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, record, &data);
  if (err != paNoError)
    return oh_no_what_now(data, err);

  //start recording
  err = Pa_StartStream(stream);
  if(err != paNoError)
    return oh_no_what_now(data, err);

  short max = 0;

  //while still recording...
  while((err = Pa_IsStreamActive(stream)) == 1){
    Pa_Sleep(50);

    //avg
    for(i=0; i<numSamples; i++){
      val = data.recAud[i];
      if(val < 0){
        //printf("1-%d\n", max);
        val = -val; //rectify
        max = max - slope;
      }

      if(val > max){
        //printf("2-%d\n", max);
        max = val; //take largest
        max = max - slope;
      }

    }

    if(max > thresh) {
      if(RX == 1) {
        RX =  0;
        data.frameIndex = 0;
        TX = 1;
        printf("Receiveing!\n");
      }
    } else if (TX == 1) {
      endat = data.frameIndex;
      TX = 0;
      printf("Transmitting!\n");
      break;
    }
  }

  if(err < 0)
    return oh_no_what_now(data, err);

  err = Pa_CloseStream(stream);
  if(err != paNoError)
    return oh_no_what_now(data, err);

  //Play audio
  data.frameIndex = 0;

  //open default device
  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device.\n");
    return oh_no_what_now(data, err);
  }

  outputParameters.channelCount = NUM_CHANNELS;
  outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(&stream, NULL, &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, play, &data );
  if (err != paNoError)
    return oh_no_what_now(data, err);

  if (stream) {
    err = Pa_StartStream(stream);
    if(err != paNoError)
      return oh_no_what_now(data, err);
    
    while ((err = Pa_IsStreamActive(stream)) == 1) {
    Pa_Sleep(25);

      if (data.frameIndex >= endat) {
        break;
      }
    }

    if (err < 0)
      return oh_no_what_now(data, err);
        
    err = Pa_CloseStream(stream);
    if (err != paNoError)
      return oh_no_what_now(data, err);
      
  }
  return 0;
}

void *repeat(void *nothingness){
  for (;;){
    sound();
  }
  return nothingness;
}
