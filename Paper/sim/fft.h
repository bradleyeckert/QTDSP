#ifndef FFT_H
#define FFT_H

#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define MAXPOW 24

struct complex
{
    double r;
    double i;
};

struct complex* InitFFT(int points);
void ByeFFT(void);
void FFT(void);
void FFTwindow(void);

#endif

