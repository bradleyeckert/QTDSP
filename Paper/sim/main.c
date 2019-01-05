#include <stdio.h>
#include "fft.h"

#define N 512

struct complex *Y; // FFT working buffer

void dumpComplex (struct complex *data, char* filename)
{
    FILE *fp;  int i;
    fp = fopen(filename, "w+");
    for (i=0; i<N; i++) {
        fprintf(fp, "%g,%g\n", Y[i].r, Y[i].i);
    }
    fclose(fp);
}

int main()
{
    double freq = 2222;
    double phase = 0;
    double fs = 10000;
    double A = 10000;
    int i;

    Y = InitFFT(N);

    for (i=0; i<N; i++) {
        Y[i].r=A*cos(2.0*PI*freq*i/fs - phase*PI/180);
        Y[i].i=0.0;
    }
    dumpComplex(Y, "Y.txt");
    FFTwindow();
    FFT();
    dumpComplex(Y, "U.txt");


    ByeFFT();
    return 0;
}
