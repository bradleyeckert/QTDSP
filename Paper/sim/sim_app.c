/*******************************************************************************
Proof of concept chirp detection by Brad Eckert. Revision 0.
Based on the paper "Quantum Time Signals in Living Beings" by Brad Eckert.
Equations and sections are cross-referenced to the paper whenever possible.
This version of the software is intended as a reference, so speed-up hacks are
avoided in favor of clarity.

Copyright 2019, Brad Eckert

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

1. This permission notice does not grant patent rights.

2. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Revision History
0: Work in progress, not officially released.
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "FFT/kiss_fft.h"
#include "tools.h"

#define N          1024         // FFT points
#define MAXPOINTS 16384         // X size
#define sigAmpl   10000         // amplitude of test chirp
#define H_V0          4         // output step size
#define H_X0         16         // input step size
//#define VERBOSE               // you probably want PASSES=1 for this
#define PASSES       25
#define PI 3.1415926538

float R = 0.5;
float frequency = 0.2;       	// initial frequency for test chirp, near Fs/2(1.0)
int pink = 0;                   // the chirp spectrum is white or pink
float gamma = 1.0/2;

float maxscale(void) {          // maximum amplitude scale
    if (R<0.0) {
        return exp(-R*N/MAXPOINTS);
    } else {
        return 1.0;
    }
}
float signal(int idx) {
	float fscale = (exp((float)idx * R / N) - 1.0) * PI * N / R;
	float ascale = exp((float)idx * 0.5 * R / N) * maxscale();
	static float angle0 = 0;
	if (pink) {
        ascale = 1.0;
	}
	float angle = frequency * fscale;
	float delta = angle - angle0;
	float z;
	angle0 = angle;
	if (delta > 3.0) {          // close to Fs/2
        z = (PI - delta) / (PI-3);
        if (z<0) {z=0;}         // taper off the amplitude
        ascale *= z;
	}
    float sig = sigAmpl * ascale * sin (angle);
    return sig;
}

float W[PASSES][N/2];          	// warped version of FFT output

///////////////////////////////////////////////////////////////////////////////
int main()
{
	float * X;                  // raw input
	float * XW;                 // warped input
    kiss_fft_cpx * Y;           // FFT input (copy of XW)
    kiss_fft_cpx * U;           // FFT result
	float * mag2;               // magnitude^2 of FFT result
	float * V;                  // correlation buffer
	float * Vout;               // output
	int VoutSize = 0;           // points in the output

    int nbytes = N * sizeof(kiss_fft_cpx);
    X=(float *)malloc(MAXPOINTS * sizeof(float));
    XW=(float *)malloc(N * sizeof(float));
    Y=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    U=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    mag2=(float *)malloc(N * sizeof(float)/2);
    V=(float *)malloc(N/2 * sizeof(float));
    Vout=(float *)malloc(MAXPOINTS * sizeof(float));

    for (int i=0; i<MAXPOINTS; i++) {
        X[i] = signal(i);       // set up a test chirp
    }
    dumpReal(X, 2*N, "X.txt");  // dump the test chirp

    kiss_fft_cfg cfg = kiss_fft_alloc(N,0,0,0 );

    CreateHann(N);              // set up the FFT window

// R and X are known, process X into V

    // M is the number of X input samples to warp to Y. Usually N to 4N.
    float M = -N * log(1 - N*(1 - exp(-fabs(R)/N))) / fabs(R);  // eq. 7
    // rate constant for downsampling exponential sweep
    double lambda = exp(-R/N) - 1;                              // eq. 9
	// downsample pitch
    float pitch = 1.0;                                          // eq. 10
    if (R>0) {
        pitch = exp(M*R/N);
    }
    // k is an upsampling constant, about 2
    double k = N * log(((float)N-2) / ((float)N-(2+2/gamma)));  // eq. 15
	// real H_X, ideal offset in X samples
	float H_V = H_X0 * fabs(R) / k;                             // eq. 17
	int H_X = H_X0;
	double zeta = exp(k/N) - 1;  // upsampling rate             // eq. 16

    printf("N=%d, M=%g, R=%g, gamma=%g, k=%g\n",N,M,R,gamma,k);
    printf("pitch=%g, lambda=%g\n", pitch, lambda);
    printf("H_X=%d, H_V=%f, zeta=%g\n",	H_X, H_V, zeta);
    double begintime = now();
    int offset = 0;

    for (int i=0; i<(N/2); i++) {           // clear V, use 1.0 so log is 0
        V[i] = 1;
    }

	int vmask = N/2 - 1;	                // one less than an exact power of 2
	int deadtime = N/2;
    float voffset = 0;
    int maxIdx = (N/2)*gamma;

    for (int p=0; p<PASSES; p++) {
		#ifdef VERBOSE
		double mark = now();
		#endif

		compress(&X[offset], XW, N, pitch, lambda, -lambda/2, 1, 0);
		#ifdef VERBOSE
		printf("Downsampled X to Y in %.3f usec\n", now() - mark);
		dumpReal(XW,N,"Y.txt");             // dump the time-warped input
		mark = now();
		#endif

		for (int i=0; i<N; i++) {           // copy real to complex
			Y[i].r = XW[i];  Y[i].i = 0.0;
		}
		HannWindow(Y);
		kiss_fft(cfg, Y, U);
		for (int i=0; i<maxIdx; i++) {
			mag2[(maxIdx-1)-i] = U[i].r * U[i].r + U[i].i * U[i].i;
		}
		#ifdef VERBOSE
		printf("Executed N-pt FFT in %.3f usec\n", now() - mark);
		dumpComplex(U,N/2,"U.txt");         // dump the FFT output
		mark = now();
		#endif

		memset(W[p],0,sizeof(W[0]));        // clear W
		compress(mag2, W[p], maxIdx, 1, -zeta, 0, 1, 1);
		#ifdef VERBOSE
		printf("Upsampled U to W in %.3f usec\n", now() - mark);
		dumpReal(W[p],N/2,"W.txt");// dump the time-warped output
		mark = now();
		#endif

		// Correlate Ws in V using an offset
		int Widxlast = maxIdx - 1;
        for (int i=0; i<=Widxlast; i++) {   // correlate
            if (R<0) {                      // R < 0
                V[vmask & ((int)voffset + i)] += W[p][Widxlast - i];
            } else {                        // R > 0
                V[vmask & ((int)voffset + i)] += W[p][i];
            }
        }
        for (int i=-H_V; i<0; i++) {        // output H_V finished points
            float dB = 10*log10( V[vmask & ((int)voffset + i)] );
//            V[vmask & ((int)voffset + i)] = 1;
            if (deadtime) {
                if (dB) {deadtime=0;}
            } else {
                Vout[VoutSize++] = dB;
            }
        }

		#ifdef VERBOSE
		printf("Correlated W to V in %.3f usec\n", now() - mark);
		mark = now();
		#endif

		offset += H_X;
		voffset += H_V;
    }
    for (int i=0; i<maxIdx; i++) {  // flush partial correlations
        float dB = 10*log10( V[vmask & ((int)H_V*PASSES + i)] );
        if (deadtime) {
            if (dB) {deadtime=0;}
        } else {
            Vout[VoutSize++] = dB;
        }
    }

    double endtime = now();

    dumpReal(V,maxIdx,"V.txt");                // correlated mag^2s
    dumpReal(Vout,VoutSize,"Vout.txt");     // output buffer in dB

/*******************************************************************************
Dump the RMS W from all of the passes to a CSV file for plotting.
*******************************************************************************/

    printf("Finished %d passes in %.3g msec.\n", PASSES, 1e-3*(endtime - begintime));

    FILE *fp;
    fp = fopen("AllW.csv", "w+");
    for (int i=0; i<maxIdx; i++) {
        for (int j=0; j<PASSES; j++) {
            fprintf(fp, "%g", sqrt(W[j][i]));
            if (j<(PASSES-1)) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    kiss_fft_free(cfg);
    FreeHann();
    free(Vout);
    free(V);
    free(mag2);
    free(U);
    free(Y);
    free(XW);
    free(X);
    return 0;
}
