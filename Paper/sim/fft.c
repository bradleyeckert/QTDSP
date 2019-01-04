// Mike Andrews -- 6/29/1998, from http://www.gweep.net/~rocko/FFT/
// FFT4 parts removed because Radix4 is only marginally faster.

#include <stdio.h>
#include "fft.h"

int pow_2[MAXPOW];				// powers-of-2 table
int N;							// points in the FFT
struct complex *data;			// working buffer, N points
double *win;                    // window function waveform

void ByeFFT(void) {				// destruct the working buffer
	free(data);
}

struct complex* InitFFT(int points)
{
    int i;
	N = points;
    pow_2[0] = 1;				// Set up power of two arrays
    for (i=1; i<MAXPOW; i++) {
        pow_2[i]=pow_2[i-1]*2; }
    win = malloc(sizeof(double) * (size_t)N);
    if (win) {
        for (i=0; i<N; i++) {   // Hann window
            win[i] = cos(2.0*PI*i/(N-1)) + 1.0;
        }
    }
	return malloc(sizeof(struct complex) * (size_t)N);
}

void FFTwindow(void)            // apply window
{
    int i;
    double w;
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
    double tempr, tempi;

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

static void twiddle(struct complex *W, int N, double stuff)
{
    W->r=cos(stuff*2.0*PI/(double)N);
    W->i=-sin(stuff*2.0*PI/(double)N);
}

/** RADIX-2 FFT ALGORITHM */
void radix2(struct complex *data, int N)
{
    int    n2, k1, N1, N2;
    struct complex W, bfly[2];

    N1=2;
    N2=N/2;

    /** Do 2 Point DFT */
    for (n2=0; n2<N2; n2++)
    {
	/** Don't hurt the butterfly */
	twiddle(&W, N, (double)n2);
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
}

