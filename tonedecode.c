#include<stdio.h>
#include<math.h>

#define pi 3.14159265358979323846264338327

//set sample rate, block size, and frequencies of interest
const int sampleRate = 16000;
const int blockln = 128;
const int windowmov = 64;
const int interp = 5;
const int freq[] = { 697, 770, 852, 941, 1209, 1336, 1477, 1633 };
int samPerSym;
int numoffreq;
char keys[4][4] = { {'1', '2', '3', 'A' }, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'} };

//used in dtmf
float toneAvg = 0;
float sum = 0;


//for sample processing
short int sam;

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
void findMag(float *fmagn, float *samp0, float *samp1, float *samp2, float *coeff) {

  //compute magnitudes of each frequency of interest
  int i;
  for(i=0; i < numoffreq; i++) {
    fmagn[i] = samp2[i]*samp2[i] + samp1[i]*samp1[i] - coeff[i]*samp1[i]*samp2[i];
  }

  for(i=0; i < numoffreq; i++){
    printf("f,%d, m,%f\n", freq[i], fmagn[i]);
  }

  //reset past sample buffers for next block
  for(i=0; i != numoffreq; i++) {
    samp0[i] = 0;
    samp1[i] = 0;
    samp2[i] = 0;
  }
}

//from intmain

char dtmf(short *audio){

  //find number of frequencies
  numoffreq = sizeof(freq)/sizeof(int);

  //allocate DFT arrays
  float normfreq[numoffreq];
  float coef[numoffreq];
  float fmag[numoffreq];
  float sam0[numoffreq];
  float sam1[numoffreq];
  float sam2[numoffreq];

  //used in counters
  int i;

  //compute normalized frequencies
  for(i = 0; i != numoffreq; i++) {
    normfreq[i] = (float)freq[i]/((float)sampleRate*((float)interp+1));
  }

  //compute coefficents
  for(i = 0; i != numoffreq; i++) {
    coef[i] = 2*cos(2*pi*normfreq[i]);
  }

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

  //instead of useing a file we need to have the function take an address of an array
  //and put the contence of the array into buffer b

  //keep analyzing if you haven't got to EOF
  //while(!feof(pfile)){
  int audioSize = sizeof(audio);  

  //for all of the data in our array that was passed, keep grabbing data.
  int k;
  for(k=0; (k*windowmov) < audioSize; k++) { 

    //read: windomov amount of, short int, once, and place in buffer b
    //fread(&readBuffer[0], sizeof(readBuffer), 1, pfile);

    for(i=0; i < windowmov && ( (k*windowmov)+i) < audioSize; i++) {
      readBuffer[i] = audio[(k*windowmov)+i];
    }

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
    
    //dtmf decision making


    for(i=0; i < numoffreq; i++){
      if (fmag[i] != fmag[i]) break;
      toneAvg += fmag[i]/(float)numoffreq;
    }

    printf("\navg: %f\n\n", toneAvg);
    //the two tones that are present for a single key will be above this average
    char col = 'e';
    char row = 'e';

    //if dtmf tones are not at the begining of the
    //freq array, these two for loops will need changed
    for(i=0; i < 4; i++){
      if(fmag[i] > toneAvg){
        row = i;
        printf("above%d", i);
      }
    }

    for(i=4; i < 8; i++){
      if(fmag[i]>toneAvg){
        col = i;
        printf("above%d", i);
      }
    }

    sum = 0;
    toneAvg = 0;

    printf("\n\n%c %d %d\n\n", keys[(int)row][(int)col], row, col);
    return keys[(int)row][(int)col];

  }

  // Should never get here?
  return -1;
}
