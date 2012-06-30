/*

 Program should eventualy

-open the configfile for simplex repeater and read into memory
-open port audio stream
-T1 above audio threshold? ID time?
-T2 called above threshold: record, once done block T1 and play back with courtesy tone
-T3 called above threshold: check for DTMF or PL


Right now:

-open audio stream for 5s
-play back

needed
recording function
playback function

*/

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (256)
#define NUM_SECONDS (4)
#define NUM_CANNELS (2)
#define DITHER_FLAG (0)
#define WRITE_TO_FILE (0)



#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

int main(int argc, char* argv[])
{

  //stream callback
  typedef int PaStreamCallback( const void *input,
                                        void *output,
                                        unsigned long frameCount,
                                        const PaStreamCallbackTimeInfo* timeInfo,
                                        PaStreamCallbackFlags statusFlags,
                                        void *userData ) ;
  int err;
  err = Pa_Initalize();
  if (err != paNoError) goto error;

  //initialize a stream
  PaStream *stream;
  PaError err;
  if (err != paNoError) goto error;

  //open a stream using default values
  err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                patestCallback, /* this is your callback function */
                                &data ); /*This is a pointer that will be passed to
                                                   your callback*/
  //open stream 
  err = Pa_StartStream( stream );
  if( err != paNoError ) goto error;

  //let the stream do its thing
  Pa_Sleep(NUM_SECONDS*1000);

  err = Pa_StopStream( stream );
  if( err != paNoError ) goto error;

  //close stream, free up reources
  err = Pa_CloseStream( stream );
  if( err != paNoError ) goto error;

  //exiting portaudio
  err = Pa_Terminate();
  if( err != paNoError ) goto error;

  error:
    printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );





