// Mike Andrews -- 6/29/1998, from http://www.gweep.net/~rocko/FFT/
// FFT4 parts removed because Radix4 is only marginally faster.

#include <stdio.h>
#include "fft.h"
#define N          1024         /* FFT points, change to int */

int pow_2[MAXPOW];				// powers-of-2 table
struct complex data[N];			// working buffer, N points
float *win;                     // window function waveform

void ByeFFT(void) {				// destruct the working buffer
}

struct complex* InitFFT(void)
{
    int i;
    pow_2[0] = 1;				// Set up power of two arrays
    for (i=1; i<MAXPOW; i++) {
        pow_2[i]=pow_2[i-1]*2; }
    win = malloc(sizeof(float) * (size_t)N);
    if (win) {
        for (i=0; i<N; i++) {   // Hann window
            win[i] = 1 - cos(2.0*PI*i/(N-1));
        }
    }
	return data;
}

void FFTwindow(void)            // apply window
{
    int i;
    float w;
    for (i=0; i<N; i++) {
        w = win[i];
        data[i].r *= w;
        data[i].i *= w;
    }
}

void FFTreorder(void)			// bit-reverse the working buffer data
{								// do this before or after the FFT
    int bits = 0;
    int i, j, k;
    float tempr, tempi;

    for (i=0; i<MAXPOW; i++)
	if (pow_2[i]==N) bits=i;

    for (i=0; i<N; i++)
    {
	j=0;
	for (k=0; k<bits; k++)
	    if (i&pow_2[k]) j+=pow_2[bits-k-1];

	if (j>i)  /** Only make "up" swaps */
	{
	    tempr = data[i].r;
	    tempi = data[i].i;
	    data[i].r = data[j].r;
	    data[i].i = data[j].i;
	    data[j].r = tempr;
	    data[j].i = tempi;
	}
    }
}

static void twiddle(struct complex *W, int size, float stuff)
{
    W->r=cos(stuff*2.0*PI/(float)size);
    W->i=-sin(stuff*2.0*PI/(float)size);
}

/** RADIX-2 FFT ALGORITHM */
void radix2(struct complex *data, int size)
{
    int    n2, k1, N1, N2;
    struct complex W, bfly[2];

    N1=2;
    N2=size/2;

    /** Do 2 Point DFT */
    for (n2=0; n2<N2; n2++)
    {

	/** Don't hurt the butterfly */
	twiddle(&W, size, (float)n2);
	bfly[0].r = (data[n2].r + data[N2 + n2].r);
	bfly[0].i = (data[n2].i + data[N2 + n2].i);
	bfly[1].r = (data[n2].r - data[N2 + n2].r) * W.r -
	    ((data[n2].i - data[N2 + n2].i) * W.i);
	bfly[1].i = (data[n2].i - data[N2 + n2].i) * W.r +
	    ((data[n2].r - data[N2 + n2].r) * W.i);

	/** In-place results */
	for (k1=0; k1<N1; k1++)
	{
	    data[n2 + N2*k1].r = bfly[k1].r;
	    data[n2 + N2*k1].i = bfly[k1].i;
	}
    }

    /** Don't recurse if we're down to one butterfly */
    if (N2!=1)
	for (k1=0; k1<N1; k1++)
	    radix2(&data[N2*k1], N2);
}

void FFT(void) {
    radix2(data, N);
    FFTreorder();
}

