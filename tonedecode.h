// Jimmy Carter - KG4SGP
// Licensed as BSD (license at bottom)

// Goertzel FSK Demodulator
// Analyses audio for tones, the demodulates
// based on the differenc in magnitude

//You can find a description of the demodulator at
//http://www.kg4sgp.com/projects/modem.html

//compile this code with "gcc goertzel.c -o goertzel -lm"

//Features:
//   Variable Block Size
//   User defined number of frequencies of interest
//   Variable Sample Rate
//   Sliding Window
//   Demodulation using user defined baud rate
//   Output that can lend its self to graphing
//   Lots of comments
//   Funny styling

//Things to do:
//   Make options editable on command line
//   Report time taken to demodulate
//   Integrate this with the audio recorder... 

//as you can see, right now
//it's pretty portable :D
#include<stdio.h>
#include<math.h>

#define pi 3.14159265358979323846264338327

//set sample rate, block size, and frequencies of interest
const int sampleRate = 48000;
const int blockln = 60;
const int windowmov = 4;
const int interp = 2;
const int freq[] = { 1300, 2100 };
int samPerSym;
int numoffreq;


//for sample processing
short int sam;
float diff;
float thresh = 5000000.0;
int baud = 1200;
int acumone = 0;
int acumzer = 0;


//declaring functions
void doBlock(float*, float*, float*, float*, float*);
void findMag();
void demod(float*);
void printInfo(float*, float*);

//does calculations for every sample
void doBlock(float* block, float* samp0, float* samp1, float* samp2, float* coeff) {

  int i;
  for(i =0; i < blockln*(interp + 1); i++) {

    //copy block to sample, not exactly efficient, but I was lazy
    sam = block[i];

    //do per sample computations for each frequency of interest
    int j = 0;
    for(j=0; j != numoffreq; j++) {
      samp0[j] = sam + coeff[j]*samp1[j] - samp2[j];
      samp2[j] = samp1[j];
      samp1[j] = samp0[j];
    }

  }
}


//finds the magnitudes of each of our frequencies of interest
//and the difference between the first two frequencies
//calls demodulator
void findMag(float* fmagn,float* samp0, float* samp1, float* samp2, float* coeff) {

  //compute magnitudes of each frequency of interest
  int i;
  for(i=0; i != numoffreq; i++) {
    fmagn[i] = samp2[i]*samp2[i] + samp1[i]*samp1[i] - coeff[i]*samp1[i]*samp2[i];
  }

  //find difference in amplitude
  diff = fmagn[0] - fmagn[1];

  //un-comment the next 2 lines
  //if you want to output a comma separated list of magnitudes
  //that you can easily graph in a spread sheet program
  //printf("%f,", fmagn[0]);
  //printf("%f,\n", fmagn[1]);

  //demodulate comment if you want a graph of magnitudes
  demod(&diff);

  //reset past sample buffers for next block
  for(i=0; i != numoffreq; i++) {
    samp0[i] = 0;
    samp1[i] = 0;
    samp2[i] = 0;
  }
}



//demodulate the transition stream
void demod(float* dif){

  int i = 0;

  //if above the threshold demodulate
  if(abs(dif) > thresh){

    //if on + swing (indicating a 1)
    if (*dif > 0) {
      //accumulate number of sample its been one
      acumone++;

      //if this is a transition
      //do calculations on previous 0s
      if(acumone == 1){
        int zs = (acumzer/samPerSym);
        int left = acumzer%samPerSym;

        
        if(left > (0.85*samPerSym)) {
          zs++;
        }

        for(i = zs; i != 0; i--) {
          printf("0");
        }

        acumzer = 0;
      }
    } else if (*dif < 0) {
      acumzer++;

      //if this is a transition
      //do calculations on previous 1s
      if(acumzer == 1){
        int os = (acumone/samPerSym);
        int left = acumone%samPerSym;

        if(left > (0.85*samPerSym)) {
          os++;
        }

        for(i = os; i != 0; i--) {
          printf("1");
        }

        acumone = 0;
      }
    }
  }
}



//prints mostly the initial constant stuff
//helpful when printing lots of result files
void printInfo(float* normalfreq, float* coeff) {
  //output some info
  printf("Sample Rate: %d\n", sampleRate);
  printf("Block Length: %d\n", blockln);
  printf("Slide increment: %d\n", windowmov);
  printf("Interpolated pts btw sam: %d\n", interp);
  printf("Effective Sample Rate: %d\n", (sampleRate*(interp+1)));

  int i;
  for(i=0; i != numoffreq; i++) {
    printf("\nfrequency %d: %d\n", i, freq[i]);
    printf("f%d normalized frequency: %f\n", i, normalfreq[i]);
    printf("f%d coefficient: %f\n", i, coeff[i]);
  }
    //demodulation stuff
    printf("\nSamples per symbol: %d\n", samPerSym);
    printf("Threshold: %f\n", thresh);
}



//computes some initial values and then analyzes the file
int main(int argc, char* argv[]) {

  //find number of frequencies
  numoffreq = sizeof(freq)/sizeof(int);

  //allocate DFT arrays
  float normfreq[numoffreq];
  float coef[numoffreq];
  float fmag[numoffreq];
  float sam0[numoffreq];
  float sam1[numoffreq];
  float sam2[numoffreq];

  //for counters
  int i;

  //compute normalized frequencies
  for(i = 0; i != numoffreq; i++) {
    normfreq[i] = (float)freq[i]/((float)sampleRate*((float)interp+1));
  }

  //compute coefficents
  for(i = 0; i != numoffreq; i++) {
    coef[i] = 2*cos(2*pi*normfreq[i]);
  }

  //compute demodulation constants
  samPerSym = ((int)(sampleRate)/(baud*windowmov));

  //print info about the current demodulator settings, if desired
  printInfo(&normfreq[0], &coef[0]);

  //buffers
  short int readBuffer[windowmov];
  short int blockBuffer[blockln];
  float interpBuffer[(int)blockln*(interp + 1)];

  //clear buffers
  for(i=0; i != blockln; i++) {
    blockBuffer[i] = 0;
  } 

  for(i=0; i != windowmov; i++) {
    readBuffer[i] = 0;
  }

  for(i=0; i != (blockln*interp); i++){
    interpBuffer[i] = 0.0;
  }

  //create file pointer
  FILE *pfile;
  
  //filename is taken as the first argument to the program
  //file is opend read-only binary
  pfile = fopen(argv[1], "rb");
  if (!pfile) {
    printf("Unable to open file\n\n");
    return 1;
  }

  //for wav files we move past header stuff... I don’t care about it
  //this is: on pfile, move 44 bytes, from the start
  fseek(pfile, 44, SEEK_SET);

  //keep analyzing if you haven't got to EOF
  while(!feof(pfile)){

    //read: windomov amount of, short int, once, and place in buffer b
    fread(&readBuffer[0], sizeof(readBuffer), 1, pfile);


    //move samples in buffer windowmov ammount to the left
    for(i=windowmov; i != blockln; i++) {
      blockBuffer[(i-(int)windowmov)] = blockBuffer[i];
    }

    //put the windowmov amount of new samples at the end of the buffer
    for(i=windowmov; i != 0; i--) {
      blockBuffer[(int)(blockln-i)] = readBuffer[(int)windowmov-i];
    }

    //interpolate in-between the samples
    for(i=0; i != blockln; i++) {

      //find the step in-between samples
      float step = (blockBuffer[i+1] - blockBuffer[i])/(interp+1);

      //interpolate between the two given samples
      int j = 0;
      for(j=0; j != (interp+1); j++) {
        interpBuffer[(i*(interp+1))+j] = blockBuffer[i]+(j)*step;
      }
    }

    //do calculations on current block of samples
    //then find the magnitude of the tones
    doBlock(&interpBuffer[0], &sam0[0], &sam1[0], &sam2[0], &coef[0]);
    findMag(&fmag[0], &sam0[0], &sam1[0], &sam2[0], &coef[0]);
  }

  //make sure you're at EOF then declare
  if(feof(pfile)){
    printf("\nReached end of file...\n\n");
  } else {
    printf("Unknown error...\n\n");
  }

  //close the file and exit
  fclose(pfile);
  return 0;
}

