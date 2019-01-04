#include <stdio.h>
#include "fft.h"

#define numsamp 1024

struct complex *Y;

int main()
{
    double freq = 5000;
    double phase = 0;
    double fs = 44100;
    double A = 10000;
    int i;

    printf("Hello world!\n");
    Y = InitFFT(numsamp);

    for (i=0; i<numsamp; i++) {
        Y[i].r=A*cos(2.0*PI*freq*i/fs - phase*PI/180);
        Y[i].i=0.0;
    }

    ByeFFT();
    return 0;
}
