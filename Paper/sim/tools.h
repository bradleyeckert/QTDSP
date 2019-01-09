#ifndef TOOLS_H
#define TOOLS_H

#include <math.h>
#include <stdio.h>
#include "fft.h"

void compress(
	float* in,			        // input stream
	float* out,		            // output stream
	int length,			        // points in output stream
	float pitch,		        // exponential input sample pitch
	float Prate,		        // growth/decay rate of pitch
	float Arate,		        // growth/decay rate of amplitude
	int post);                  // step pitch upon output

void dumpComplex (struct complex *data, int length, char* filename);
void dumpReal (float *data, int length, char* filename);
double now();                   // current time in usec

void CreateHann(int length);    // create a window vector
void FreeHann(void);            // free it
void HannWindow(struct complex *data); // apply the window

#endif

