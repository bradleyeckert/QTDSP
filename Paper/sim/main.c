#include <stdio.h>
#include <string.h>
#include "fft.h"

#define N          512          // FFT points
#define MAXPOINTS 4096          // X size
#define sigAmpl  10000          // amplitude of test chirp
#define H_V0         1          // output step size
#define VERBOSE

double X[MAXPOINTS];            // X input buffer
double R = -0.5;
double frequency = 0.9;         // initial frequency for test chirp
int pink = 0;                   // the chirp spectrum is white or pink

double now();

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
	if (pink) {
        ascale = 1.0;
	}
	float angle = frequency * fscale;
    float sig = sigAmpl * ascale * sin (angle);
    return sig;
}

/* Compress
Interpolate LENGTH output samples from a sequence of input samples.
Pitch is stepped after each output point is stored if `post`=1.
*/

void compress(
	double* in,			        // input stream
	double* out,		        // output stream
	int length,			        // points in output stream
	double pitch,		        // exponential input sample pitch
	double Prate,		        // growth/decay rate of pitch
	double Arate,		        // growth/decay rate of amplitude
	int post)                   // step pitch upon output
{
	int oc = length;	        // output counter
	double idx0 = 0;	        // input index
	double idx1 = 0;
	int pending = 1;	        // input read is pending
	double X0 = 0;
	double X1 = *in++;
	double Y;
	double Ascale = 1.0;        // amplitude compensation
	while (oc) {
        if (pending) {
            pending = 0;
            X0 = X1;
            X1 = *in++;
            if (post==0) {      // sweep with the input
                pitch += pitch * Prate;
            }
        }
        double frac0, int0;     // integer and fractional parts of X indices
        double frac1, int1;
		idx0 = idx1;            // next input span
		idx1 += pitch;
        frac0 = modf(idx0, &int0);
        frac1 = modf(idx1, &int1);
		int k = int1 - int0;    // input span crosses this many boundaries
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
        *out++ = Y * Ascale;  oc--;
        if (post) {             // sweep with the output
            pitch += pitch * Prate;
        }
        Ascale += Ascale * Arate;
	}
}


void dumpComplex (struct complex *data, int length, char* filename)
{
    FILE *fp;  int i;
    fp = fopen(filename, "w+");
    for (i=0; i<length; i++) {
        fprintf(fp, "%g,%g\n", data[i].r, data[i].i);
    }
    fclose(fp);
}

void dumpReal (double *data, int length, char* filename)
{
    FILE *fp;  int i;
    fp = fopen(filename, "w+");
    for (i=0; i<length; i++) {
        fprintf(fp, "%g\n", data[i]);
    }
    fclose(fp);
}

double XW[N];                   // warped version of X input
double mag2[N/2];
double W[N/2];                  // warped version of FFT output
double V[N];                    // Correlated passes

int main()
{
    struct complex *Y;          // FFT working buffer of N points
    float M;                    // input samples in Y
    float k;                    // upsampling constant, about 2
    int i;

    memset(V, 0, sizeof(V));    // clear V

    for (i=0; i<MAXPOINTS; i++) {
        X[i] = signal(i);       // set up a test chirp
    }
    dumpReal(X, 2*N, "X.txt");  // dump the test chirp
    Y = InitFFT(N);             // set up Y for in-place FFT

    M = -N * log(1 - N*(1 - exp(-fabs(R)/N))) / fabs(R);
    k = N * log(((float)N-2)/((float)N-4));

    double lambda = exp(-R/N) - 1;
    float pitch = 1.0;
    if (R>0) {
        pitch = R*M/N;          // upchirp starts sample pitch at e^RM/N
    }

    printf("N=%d, M=%g, R=%g, k=%g, lambda=%g\n",N,M,R,k,lambda);

    int offset = 0;

    #ifdef VERBOSE
    double mark = now();
    #endif

    compress(&X[offset], XW, N, pitch, lambda, -lambda/2, 0);

    #ifdef VERBOSE
    printf("Downsampled X to Y in %.3f usec\n", now() - mark);
    dumpReal(XW, N, "Y.txt");   // dump the time-warped input
    mark = now();
    #endif

    for (i=0; i<N; i++) {       // copy
        Y[i].r = XW[i];  Y[i].i = 0.0;
    }
    FFTwindow();                // Hann window
    FFT();
    for (i=0; i<(N/2); i++) {
        mag2[(N/2-1)-i] = Y[i].r * Y[i].r + Y[i].i * Y[i].i;
    }

    #ifdef VERBOSE
    printf("Executed FFT in %.3f usec\n", now() - mark);
    #endif

    float gamma = H_V0 * k / fabs(R); // real H_X, offset in X samples
    int H_X = round(gamma);
    int H_V = H_V0;
    float upsam_correct = gamma / H_X;
    double zeta = exp(k/N) - 1;

    #ifdef VERBOSE
    printf("H_X=%d, H_V=%d, gamma=%g, zeta=%g, corr=%g\n",
        H_X, H_V, gamma, zeta, upsam_correct);
    mark = now();
    #endif

    memset(W, 0, sizeof(W));    // clear V
    compress(mag2, W, N/2, 1, -zeta, 0, 1);

    #ifdef VERBOSE
    printf("Upsampled U to W in %.3f usec\n", now() - mark);
    dumpReal(W, N/2, "W.txt");   // dump the time-warped output
    #endif

// For a down-chirp, the peak in W will move right as `offset` increases.
// V is an accumulation of p W results with W shifted right by
// (pmax - p)*H_V points.

    ByeFFT();
    return 0;
}
