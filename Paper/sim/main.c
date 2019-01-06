#include <stdio.h>
#include "fft.h"

#define N          512          // FFT points
#define MAXPOINTS 4096          // X size
#define sigAmpl  10000          // amplitude of test chirp
#define H_V0         1          // output step size

double X[MAXPOINTS];            // X input buffer
double R = -0.5;
double frequency = 0.9;         // initial frequency for test chirp

int pink = 0;                   // the chirp spectrum is white or pink

float maxscale(void) {          // maximum amplitude scale
    if (R<0.0) {
        return exp(-R*N/MAXPOINTS);
    } else {
        return 1.0;
    }
}
float signal(int idx) {
	float fscale = (exp((float)idx * R / N) - 1.0) * 3.14159265 * N / R;
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

int main()
{
    struct complex *Y;          // FFT working buffer of N points
    float M;                    // input samples in Y
    float k;                    // upsampling constant, about 2
    int i;

    Y = InitFFT(N);             // set up Y for in-place FFT
    M = -N * log(1 - N*(1 - exp(-fabs(R)/N))) / fabs(R);
    k = N * log(((float)N-2)/((float)N-4));

    for (i=0; i<MAXPOINTS; i++) {
        X[i] = signal(i);       // set up a test chirp
    }
    dumpReal(X, 2*N, "X.txt");  // dump the test chirp

    double lambda = exp(-R/N) - 1;
    float pitch = 1.0;
    if (R>0) {
        pitch = R*M/N;          // upchirp starts sample pitch at e^RM/N
    }

    float gamma = H_V0 * k / fabs(R);    // real H_X, offset in X samples

    printf("N=%d, M=%g, R=%g, k=%g, lambda=%g, H_X=%g\n",N,M,R,k,lambda,gamma);

    compress(X, XW, N, pitch, lambda, -lambda/2, 0);
    dumpReal(XW, N, "Y.txt");   // dump the time-warped input

    for (i=0; i<N; i++) {       // copy
        Y[i].r = XW[i];  Y[i].i = 0.0;
    }
    FFTwindow();                // Hann window
    FFT();
    dumpComplex(Y, N, "U.txt"); // FFT result before upsampling
// Note: You can pull U.txt into a spreadsheet to calculate and plot amplitudes

    ByeFFT();
    return 0;
}
