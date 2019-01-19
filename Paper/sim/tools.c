/* Tools for QTDSP, excluding FFT */

#include <stdio.h>
#include <stdint.h>
#include "tools.h"
#include "./FFT/kiss_fft.h"

/** Compress
Interpolate LENGTH output samples from a sequence of input samples.
Pitch is stepped after each output point is stored if `post`=1.
Otherwise, it's stepped after each input point is read.
The pitch only needs 2 or 3 bits in the integer part.
Although more bits of precision are available, UQ8.24 is used to
simulate the use of a 24-bit multiplier in hardware implementations.
*/

void compress(
	float* in,			        // input stream
	float* out,		            // output stream
	int length,			        // points in output stream
	float pitch,		        // exponential input sample pitch
	float Prate,		        // growth/decay rate of pitch
	float Arate,		        // growth/decay rate of amplitude
	int post)                   // step pitch upon output
{
    pitch *= (float)0x1000000L; // indices use UQ8.24 format
    float fs = 1/(float)0x1000000L;
	uint32_t idx0 = 0;	        // input index
	uint32_t idx1 = 0;
	int pending = 1;	        // input read is pending
	float X0 = 0;
	float X1 = *in++;
	float Y;
	float Ascale = 1.0;         // amplitude compensation
	while (length) {
        if (pending) {
            pending = 0;
            X0 = X1;
            X1 = *in++;
            if (post==0) {      // sweep with the input
                pitch += pitch * Prate;
            }
        }
		idx0 = idx1;            // next input span
		idx1 += round(pitch);
		// scale the fractional parts of indices to between 0 and 1
        float frac0 = fs * (float)(0xFFFFFF & idx0);
        float frac1 = fs * (float)(0xFFFFFF & idx1);
        // input span crosses k boundaries, difference between integer parts
		uint32_t k = 0xFF & ((idx1>>24) - (idx0>>24));
		switch (k) {
        case 0:
            Y = X0 * (frac1 - frac0);
            break;
        case 1:
            Y = X0 * (1 - frac0) + X1 * frac1;
            pending = 1;
            break;
        default: // k>1
            Y = X0 * (1 - frac0) + X1;
            while (k>1) {
                X0 = X1;
                X1 = *in++;
                if (post==0) {  // sweep with the input
                    pitch += pitch * Prate;
                }
                k--;
                if (k>1) {
                    Y += X1;
                }
            }
            Y += X1 * frac1;
            pending = 1;
            break;
		}
        *out++ = Y * Ascale;
        if (post) {             // sweep with the output
            pitch += pitch * Prate;
        }
        Ascale += Ascale * Arate;
        length--;
	}
}

float *win;                     // window function waveform
int HannLength;

void CreateHann(int length) {
    win = malloc(sizeof(float) * (size_t)length);
    HannLength = length;
    int i;
    if (win) {                  // Hann window function
        for (i=0; i<length; i++) {
            win[i] = 1 - cos(2.0*PI*i/(length-1));
        }
    }
}
void FreeHann(void) {
    free(win);
}

void HannWindow(kiss_fft_cpx *data)
{
    int i;
    float w;
    for (i=0; i<HannLength; i++) {
        w = win[i];
        data[i].r *= w;
        data[i].i *= w;
    }
}

/// Utilities

void dumpComplex (kiss_fft_cpx *data, int length, char* filename)
{
    FILE *fp;  int i;
    fp = fopen(filename, "w+");
    for (i=0; i<length; i++) {
        fprintf(fp, "%g,%g\n", data[i].r, data[i].i);
    }
    fclose(fp);
}

void dumpReal (float *data, int length, char* filename)
{
    FILE *fp;  int i;
    fp = fopen(filename, "w+");
    for (i=0; i<length; i++) {
        fprintf(fp, "%g\n", data[i]);
    }
    fclose(fp);
}

#if defined(WIN32) || defined(__WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_)

#include <windows.h>
double now() // current usec
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return 1e6*(double)t.QuadPart/(double)f.QuadPart;
}

#else

#include <sys/time.h>
#include <sys/resource.h>

double now() // current usec
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return 1e6*t.tv_sec + t.tv_usec;
}

#endif

