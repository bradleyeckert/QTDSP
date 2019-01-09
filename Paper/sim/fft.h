#ifndef FFT_H
#define FFT_H

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#define PI 3.14159265358979323846
#define MAXPOW       24

struct complex
{
    float r;
    float i;
};

struct complex* InitFFT(void);
void ByeFFT(void);
void FFT(void);

#endif

