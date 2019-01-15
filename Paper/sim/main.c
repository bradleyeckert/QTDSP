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
#include "kiss_fft.h"
#include "tools.h"

#define N          1024         // FFT points
#define MAXPOINTS 16384         // X size
#define sigAmpl   10000         // amplitude of test chirp
#define H_V0          4         // output step size
//#define VERBOSE               // you probably want PASSES=1 for this
#define PASSES       32
#define PI 3.1415926538

float X[MAXPOINTS];             // X input buffer
float R = 0.515;
float frequency = 0.4;       	// initial frequency for test chirp, near Fs/2(1.0)
int pink = 0;                   // the chirp spectrum is white or pink

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

float XW[N];                   	// warped version of X input
float mag2[N/2];
float W[PASSES][N/2];          	// warped version of FFT output
float V[N];                    	// Correlated passes

///////////////////////////////////////////////////////////////////////////////
int main()
{
    int i, p;
	int Nless1 = N - 1;			// one less than an exact power of 2
    kiss_fft_cpx * Y;
    kiss_fft_cpx * U;
    int nbytes = N * sizeof(kiss_fft_cpx);
    Y=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    U=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);

    for (i=0; i<MAXPOINTS; i++) {
        X[i] = signal(i);       // set up a test chirp
    }
    dumpReal(X, 2*N, "X.txt");  // dump the test chirp

    kiss_fft_cfg cfg = kiss_fft_alloc(N,0,0,0 );

    CreateHann(N);              // set up the FFT window

    // M is the number of X input samples to warp to Y. Usually N to 4N.
    float M = -N * log(1 - N*(1 - exp(-fabs(R)/N))) / fabs(R);  // eq. 7
    // rate constant for downsampling exponential sweep
    float lambda = exp(-R/N) - 1;                               // eq. 9
    // k is an upsampling constant, about 2
    float k = N * log(((float)N-2)/((float)N-4));               // eq. 15
	// real H_X, ideal offset in X samples
	float gamma = H_V0 * k / fabs(R);                           // eq. 17
	int H_X = round(gamma);		// use the closest value
	int H_V = H_V0;
	float upsam_correct = (gamma / H_X) - 1;
	float zeta = exp(k/N) - 1;  // upsampling rate              // eq. 16

    float pitch = 1.0;                                          // eq. 10
    if (R>0) {
        pitch = exp(R*M/N);     // upchirp starts sample pitch at e^RM/N
    }

    printf("N=%d, M=%g, R=%g, k=%g\n",N,M,R,k);
    printf("pitch=%g, lambda=%g\n",pitch,lambda);
    printf("H_X=%d, H_V=%d, gamma=%g, zeta=%g, Ucorr=%g\n",
			H_X, H_V, gamma, zeta, upsam_correct);
    double begintime = now();
    int offset = 0;
    memset(V, 0, sizeof(V));    // clear V

    for (p=0; p<PASSES; p++) {
		#ifdef VERBOSE
		double mark = now();
		#endif

		compress(&X[offset], XW, N, pitch, lambda, -lambda/2, 0);
		#ifdef VERBOSE
		printf("Downsampled X to Y in %.3f usec\n", now() - mark);
		dumpReal(XW,N,"Y.txt"); // dump the time-warped input
		mark = now();
		#endif

		for (i=0; i<N; i++) {   // copy
			Y[i].r = XW[i];  Y[i].i = 0.0;
		}
		HannWindow(Y);
		kiss_fft(cfg, Y, U);
		for (i=0; i<(N/2); i++) {
			mag2[(N/2-1)-i] = U[i].r * U[i].r + U[i].i * U[i].i;
		}
		#ifdef VERBOSE
		printf("Executed N-pt FFT in %.3f usec\n", now() - mark);
		dumpComplex(U,N/2,"U.txt"); // dump the FFT output
		mark = now();
		#endif

		memset(W[p],0,sizeof(W[0]));  // clear V
		compress(mag2, W[p], N/2 - H_V, 1, -zeta*(1+2*upsam_correct), 0, 1);
		#ifdef VERBOSE
		printf("Upsampled U to W in %.3f usec\n", now() - mark);
		dumpReal(W[p],N/2,"W.txt");// dump the time-warped output
		mark = now();
		#endif

		// Correlate Ws in V using an offset
		if (R<0) {
            for (i=0; i<(N/2-H_V); i++) {
                V[Nless1 & (i - H_V*p)] += W[p][i];
            }
		} else {
            for (i=0; i<(N/2-H_V); i++) {
                V[Nless1 & (i + H_V*p)] += W[p][i];
            }
		}
		#ifdef VERBOSE
		printf("Correlated W to V in %.3f usec\n", now() - mark);
		mark = now();
		#endif

		offset += H_X;
    }
    double endtime = now();

// dump the final output, which is square of magnitude. Take sqrt to get RMS.
    dumpReal(V,N,"V.txt");

/*******************************************************************************
Dump the RMS W from all of the passes to a CSV file for plotting.
*******************************************************************************/

    printf("Finished %d passes in %.3g msec.\n", p, 1e-3*(endtime - begintime));

    FILE *fp;  int j;
    fp = fopen("AllW.csv", "w+");
    for (i=0; i<N/2; i++) {
        for (j=0; j<p; j++) {
            fprintf(fp, "%g", sqrt(W[j][i]));
            if (j<(p-1)) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    kiss_fft_free(cfg);
    FreeHann();
    free(Y);
    free(U);
    return 0;
}
