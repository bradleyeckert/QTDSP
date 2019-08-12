#ifndef TOOLS_H
#define TOOLS_H

#include <math.h>
#include <stdio.h>
#include "FFT/kiss_fft.h"

#define PI 3.1415926538

void compress(
	float* in,			        // input stream
	float* out,		            // output stream
	int length,			        // points in output stream
	float pitch,		        // exponential input sample pitch
	float Prate,		        // growth/decay rate of pitch
	float Arate,		        // growth/decay rate of amplitude
	float Ascale,		        // amplitude compensation
	int post);                  // step pitch upon output

void dumpComplex (kiss_fft_cpx *data, int length, char* filename);
void dumpReal (float *data, int length, char* filename);
double now();                   // current time in usec

void CreateHann(int length);    // create a window vector
void FreeHann(void);            // free it
void HannWindow(kiss_fft_cpx *data); // apply the window

extern int comp_filtered;

#endif

