// from https://www.math.wustl.edu/~victor/mfmm/fourier/fft.c

/* Factored discrete Fourier transform, or FFT, and its inverse iFFT */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#define q	10
#define N	(1<<q)		/* N-point FFT, iFFT */

// Parameters for the test file
#define filename "chirp.csv"
double k1 = 2.7;      // frequency scale, width of result near full
double k2 = 2.0 / (double)N; // asymptotic freq scale, doesn't change the shape much
double k3 = 1.0;      // amplitude loss scale, 1.0 for flat.
double vert = 1000.0; // vertical scale

typedef float real;
typedef struct{real Re; real Im;} complex;

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif

/*
   fft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute fft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = -sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void
fft( complex *v, int n, complex *tmp )
{
    if(n>1) {			/* otherwise, do nothing and return */
        int k,m;    complex z, w, *vo, *ve;
        ve = tmp; vo = tmp+n/2;
        for(k=0; k<n/2; k++) {
            ve[k] = v[2*k];
            vo[k] = v[2*k+1];
        }
        fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
        fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
        for(m=0; m<n/2; m++) {
            w.Re = cos(2*PI*m/(double)n);
            w.Im = -sin(2*PI*m/(double)n);
            z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
            z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
            v[  m  ].Re = ve[m].Re + z.Re;
            v[  m  ].Im = ve[m].Im + z.Im;
            v[m+n/2].Re = ve[m].Re - z.Re;
            v[m+n/2].Im = ve[m].Im - z.Im;
        }
    }
}

/*
   ifft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute ifft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute ifft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void
ifft( complex *v, int n, complex *tmp )
{
    if(n>1) {			/* otherwise, do nothing and return */
        int k,m;    complex z, w, *vo, *ve;
        ve = tmp; vo = tmp+n/2;
        for(k=0; k<n/2; k++) {
            ve[k] = v[2*k];
            vo[k] = v[2*k+1];
        }
        ifft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
        ifft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
        for(m=0; m<n/2; m++) {
            w.Re = cos(2*PI*m/(double)n);
            w.Im = sin(2*PI*m/(double)n);
            z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
            z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
            v[  m  ].Re = ve[m].Re + z.Re;
            v[  m  ].Im = ve[m].Im + z.Im;
            v[m+n/2].Re = ve[m].Re - z.Re;
            v[m+n/2].Im = ve[m].Im - z.Im;
        }
    }
}

// -----------------------------------------------------------------------------

/* Print a vector of complexes as ordered pairs. */
// unused
static void
print_vector(
        const char *title,
        complex *x,
        int n)
{
    int i;
    printf("%s (dim=%d):", title, n);
    for(i=0; i<n; i++ ) printf(" %5.2f,%5.2f ", x[i].Re,x[i].Im);
    putchar('\n');
}


int main(void) {
    complex v[N], v0[N], scratch[N]; // v is the test signal
    int t;
    double mag, angle;

    for (t=0; t<N; t++) {               // set up a chirp
        angle = k1 * t * (1.0 - exp(-k2*t)); // approach k1 angle step size from 0
        mag = exp(-k2*k3*t);                 // amplitude scale = 1 downto 0
        v[t].Re = vert * mag * sin(angle);
        v[t].Im = 0.0;
    }

//      /* FFT, iFFT of v[]: */
//  print_vector("Orig", v, N);
//  fft( v, N, scratch );
//  print_vector(" FFT", v, N);
//  ifft( v, N, scratch );
//  print_vector("iFFT", v, N);

    memcpy(v0, v, sizeof(v));           // save v for listing
    fft( v, N, scratch );               // perform FFT, leave result in v

    FILE *ofp;
    ofp = fopen(filename, "w");
    if (ofp == NULL) {
        return 1;
    }
    for (t=0; t<N; t++) {
        mag = sqrt(v[t].Re * v[t].Re  +  v[t].Im * v[t].Im);
        fprintf(ofp, "%d,%5.2f,%5.2f\n", t, v0[t].Re, mag); // index, signal, magnitude
    }
    fclose(ofp);
    exit(EXIT_SUCCESS);
}

