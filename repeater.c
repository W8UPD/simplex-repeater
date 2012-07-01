
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <pthread.h>
#include <unistd.h>

#define SAMPLE_RATE  (8000)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (60)
#define NUM_CHANNELS    (1)
#define DITHER_FLAG     (0) 

#define WRITE_TO_FILE   (0)

#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"

typedef struct
{
  int          frameIndex;  //Where one is in array
  int          maxFrameIndex; //Length of array
  SAMPLE      *recAud; //recorded audio samples
}
  paData;

static int record( const void *inBuf, void *outBuf,
		   unsigned long framesPerBuffer,
		   const PaStreamCallbackTimeInfo* timeInfo,
		   PaStreamCallbackFlags statusFlags,
		   void *userData )
{
  paData *data = (paData*)userData;
  const SAMPLE *prwrite = (const SAMPLE*)inBuf;
  SAMPLE *pwrite = &data->recAud[data->frameIndex * NUM_CHANNELS];
  long framesToCalc;
  long i;
  int finished;
  unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

  (void) outBuf;
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  if( framesLeft < framesPerBuffer )
    {
      framesToCalc = framesLeft;
      finished = paComplete;
    }
  else
    {
      framesToCalc = framesPerBuffer;
      finished = paContinue;
    }

  if( inBuf == NULL )
    {
      for( i=0; i<framesToCalc; i++ )
	{
	  *pwrite++ = SAMPLE_SILENCE;  /* left */
	}
    }
  else
    {
      for( i=0; i<framesToCalc; i++ )
	{
	  *pwrite++ = *prwrite++;  /* left */
	}
    }

  data->frameIndex += framesToCalc;
  return finished;
}

static int play( const void *inBuf, void *outBuf,
		 unsigned long framesPerBuffer,
		 const PaStreamCallbackTimeInfo* timeInfo,
		 PaStreamCallbackFlags statusFlags,
		 void *userData )
{
  paData *data = (paData*)userData;
  SAMPLE *prwrite = &data->recAud[data->frameIndex * NUM_CHANNELS];
  SAMPLE *pwrite = (SAMPLE*)outBuf;
  unsigned int i;
  int finished;
  unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

  (void) inBuf;
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  if( framesLeft < framesPerBuffer )
    {
      //last buffer read
      for( i=0; i<framesLeft; i++ )
	{
	  *pwrite++ = *prwrite++;
	}

      for( ; i<framesPerBuffer; i++ )
	{
	  *pwrite++ = 0;
	}
      data->frameIndex += framesLeft;
      finished = paComplete;
    }
  else
    {
      for( i=0; i<framesPerBuffer; i++ )
	{
	  *pwrite++ = *prwrite++;  /* left */
	}
      data->frameIndex += framesPerBuffer;
      finished = paContinue;
    }

  return finished;
}

int useit() {
  //some starting stuff
  PaStreamParameters inputParameters, outputParameters;
  PaStream* stream;
  PaError err = paNoError;
  paData data;
  int i;
  int totalFrames;
  int numSamples;
  int numBytes;
  SAMPLE val;
  int TX = 0;
  int RX = 1;
  int thresh = 15000;
  int startat = 0;
  int endat = 0;
  int slope = 2;


  //more starting stuff
  data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; //Record for...
  data.frameIndex = 0; //zero position
  numSamples = totalFrames * NUM_CHANNELS;
  numBytes = numSamples * sizeof(SAMPLE);

  //allocate memory for recorded audio
  data.recAud = (SAMPLE *) malloc( numBytes ); /* recAud is initialised. */
  if( data.recAud == NULL )
    {
      printf("Could not allocate record array.\n");
      goto done;
    }

  //clear data.recAud array
  for( i=0; i<numSamples; i++ ) data.recAud[i] = 0;

  //initialize portAudio
  err = Pa_Initialize();
  if( err != paNoError ) goto done;

  //use the default device
  inputParameters.device = Pa_GetDefaultInputDevice(); 
  if (inputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default input device.\n");
    goto done;
  }

  //tell portaudio the parameters for the device
  inputParameters.channelCount = NUM_CHANNELS;
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  //open stream for recording and playback
  err = Pa_OpenStream(
		      &stream,
		      &inputParameters,
		      NULL,                  /* &outputParameters, */
		      SAMPLE_RATE,
		      FRAMES_PER_BUFFER,
		      paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		      record,
		      &data );
  if( err != paNoError ) goto done;

  //start recording
  err = Pa_StartStream( stream );
  if( err != paNoError ) goto done;

  SAMPLE max = 0;
  //while still recording...
  while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    {
      Pa_Sleep(50);
      //avg
      for( i=0; i<numSamples; i++ )
	{
	  val = data.recAud[i];
	  if( val < 0 ){
	    //printf("1-%d\n", max);
	    val = -val; //rectify
	    max = max - slope;
	  }
	  if( val > max ){
	    //printf("2-%d\n", max);
	    max = val; //take largest
	    max = max - slope;
	  }
	}

      if(max > thresh) {
        if(RX == 1) {
          RX =  0;
          data.frameIndex = 0;
          startat = data.frameIndex;
          TX = 1;
          printf("Transmitting: %d\n", data.frameIndex);
        }
      } else if (TX == 1) {
        endat = data.frameIndex;
        TX = 0;
        printf("Receiveing: %d\n", data.frameIndex);
        break;
      }
    }

  if( err < 0 ) goto done;

  err = Pa_CloseStream( stream );
  if( err != paNoError ) goto done;

  //Play audio
  printf("S %d, E %d\n", startat, endat);
  data.frameIndex = 0;

  //open default device
  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device.\n");
    goto done;
  }

  outputParameters.channelCount = NUM_CHANNELS;                     /* stereo output */
  outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  printf("Sending audio...\n");
  err = Pa_OpenStream(
		      &stream,
		      NULL, /* no input */
		      &outputParameters,
		      SAMPLE_RATE,
		      FRAMES_PER_BUFFER,
		      paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		      play,
		      &data );
  if( err != paNoError ) goto done;

  if( stream )
    {
      err = Pa_StartStream( stream );
      if( err != paNoError ) goto done;
        
      printf("Sending audio...\n");

      while( ( err = Pa_IsStreamActive( stream ) ) == 1 ){
	Pa_Sleep(25);
	if (data.frameIndex >= endat){
	  break;
	}
      }

      if( err < 0 ) goto done;
        
      err = Pa_CloseStream( stream );
      if( err != paNoError ) goto done;
        
    } 

 done:
  Pa_Terminate();
  if (data.recAud)       /* Sure it is NULL or valid. */
    free( data.recAud );
  if( err != paNoError )
    {
      fprintf( stderr, "An error occured while using the portaudio stream\n" );
      fprintf( stderr, "Error number: %d\n", err );
      fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
      err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
  return err;
}

int main(void)
{
  for (;;){
    useit();
  }
  return 0;
}
