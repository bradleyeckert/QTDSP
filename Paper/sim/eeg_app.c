/*******************************************************************************
Proof of concept QTDSP demodulator by Brad Eckert. Revision 0.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

1. This permission notice does not grant patent rights.

2. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Revision History
0: Work in progress, not officially released.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "tools.h"
#include "biquad.h"
#define MAXPOINTS 0x100000L
//#define RADIAL

char header[128];

void TestPattern(void) {
	// fill image with a test pattern
    for(int i=200; i<1000; i++) {
    for(int j=0; j<(2*i); j++) {
        PlotPixel(200, i<<6, (float)j*16380.0/(float)i);
    }}
}

float Number(char* str) {	// convert string to floating point number
	float d;
	sscanf(str, "%f", &d);
	return d;
}

/**
**/
char outfilename[256] = "img.bmp";
char infilename[256] = "eeg.txt";

int main(int argc, char *argv[])
{
    int m = 3;              // decimation factor
    int N = 1024;
    int H_X0 = 16;
    float MaxR = -0.20;
    float MinR = -0.78;
    int Rsteps = 800;
    int samplerate = 250;
    int offset = 0;         // offset of starting point in file
    char autoCeil = 1;
    char autoFloor = 1;
    char verbose = 0;

	float * X;              // raw input
	float * XW;             // warped input
    kiss_fft_cpx * Y;       // FFT input (copy of XW)
    kiss_fft_cpx * U;       // FFT result
	float * mag2;           // magnitude^2 of FFT result
	float * W;              // warped mag^2
	float * V;              // correlation buffer
	float * Vout;           // output

/// Overwrite defaults using command line arguments
	int Arg = 1;
	while (argc>Arg) {		// parse single-character arguments
		if (strlen(argv[Arg]) == 1) {
			switch(argv[Arg++][0]) {
				case 'b':	// set bmp filename
					strcpy(outfilename, argv[Arg++]); break;
				case 'i':	// set input filename
					strcpy(infilename, argv[Arg++]); break;
				case 'm':	// set new m
					m = (int)Number(argv[Arg++]); break;
					if (m<1 || m>100) {
                        fprintf(stderr, "m range is 1 to 100\n");
                        return 4;
					}
				case 'o':	// set new offset
					offset = (int)Number(argv[Arg++]); break;
					if (offset<0) {
                        fprintf(stderr, "o: Offset can't be negative\n");
                        return 4;
					}
				case 'N':	// set new N
					N = (int)Number(argv[Arg++]);
					if ((N-1)&N) {
                        fprintf(stderr, "N value must be a power of 2\n");
                        return 4;
					}
					if (N>8192 || N<128) {
                        fprintf(stderr, "N range is 128 to 8192\n");
                        return 4;
					}
					break;
				case 'r':	// set new Rmin
					MinR = Number(argv[Arg++]); break;
				case 'R':	// set new Rmax
					MaxR = Number(argv[Arg++]); break;
				case 's':	// set new R steps
					Rsteps = (int)Number(argv[Arg++]); break;
					if (Rsteps<1 || Rsteps>4096) {
                        fprintf(stderr, "s: R must have 1 to 4096 steps\n");
                        return 4;
					}
				case 'x':	// set new X step size
					H_X0 = (int)Number(argv[Arg++]); break;
					if (H_X0<4 || H_X0>100) {
                        fprintf(stderr, "v: H_X range is 4 to 100\n");
                        return 4;
					}
				case 'f':	// set floor of heatmap
					floorColor = Number(argv[Arg++]);
                    autoFloor = 0;	break;
				case 'c':	// set ceiling of heatmap
					ceilColor = Number(argv[Arg++]);
                    autoCeil = 0;	break;
                case 'h':   // set BMP height to multiple of 4
                    IMG_H = (3 + (int)Number(argv[Arg++])) & 0xFFFC;  break;
                case 'v':   verbose = 1;  break;
				default: break;
			}
		}
	}
    if (fabs(MinR) > 0.78 || fabs(MaxR) > 0.78) {
        fprintf(stderr, "Max |R| is 0.78\n");  return 4; }
    if ((MinR*MaxR)<0) {
        fprintf(stderr, "Min R and Min R must be of the same sign\n");
        return 4;      }
    if (fabs(MinR) > fabs(MaxR)) {  // if the R range is backward, fix it
        float temp = MinR;  MinR = MaxR;  MaxR = temp;
    }
    #ifndef RADIAL
        Rsteps = IMG_H; // override number of steps if rectangular format
    #endif // RADIAL

    double startTime = now();
	X = malloc(sizeof(float) * MAXPOINTS);
	if (BMPalloc() || (X==0)) { return 1; }     // memory error

/// Input a text file consisting of a header line and one number per line
	printf("Loading X from %s, ", infilename);
    FILE *ifp;
    ifp = fopen(infilename, "r");
    if (ifp == NULL) {
        fprintf(stderr, "Can't open input file %s\n", infilename);
        return 2;
    }
    biquad LowPass;
    BiQuad_new(LPF, 0, 0.45, m, 1.5, &LowPass); // set up decimation filter
    BiQuad_clear(&LowPass);

    fscanf(ifp, "%d", &samplerate);             // 1st line is sample rate
    fscanf(ifp, "%s", header);                  // and header string
    int j = m;
    uint32_t Xlength = 0;
    while (Xlength<MAXPOINTS) {
        float x, xf;
        if (1!=fscanf(ifp, "%f", &x)) break;    // floating point numbers
        xf = BiQuad(x, &LowPass);               // get filtered version
        if (offset) {
            offset--;                           // skip 0 or more input points
        } else {
            if (j==1) {
                X[Xlength++] = xf;              // store decimated input
                j = m;
            } else {
                j--;
            }
        }
    }
    fclose(ifp);
	printf("%d points of %s data, Fs=%dHz\n", Xlength*m, header, samplerate);
	printf("Decimation = %d, X = %d points, R = %.3f to %.3f\n",
        m, Xlength, MinR, MaxR);

/// Output decimated X to "decimated.txt" for sanity checking
	FILE *ofp;
	ofp = fopen("decimated.txt", "w");
    if (ofp != NULL) {
        for(uint32_t i=0; i<Xlength; i++) {
            fprintf(ofp, "%g\n", X[i]);
        }
    }

    int nbytes = N * sizeof(kiss_fft_cpx);
    XW=(float *)malloc(N * sizeof(float));
    Y=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    U=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    mag2=(float *)malloc(N * sizeof(float)/2);
    W=(float *)malloc(N/2 * sizeof(float));
    V=(float *)malloc(N/2 * sizeof(float));
    Vout=(float *)malloc(MAXPOINTS * sizeof(float));

    kiss_fft_cfg cfg = kiss_fft_alloc(N,0,0,0 );
    CreateHann(N);                              // set up the FFT window
	int vmask = (N/2) - 1;			// one less than an exact power of 2

    // k is an upsampling constant, about 2
    float k = N * log(((float)N-2)/((float)N-4));                   // eq. 15
    float zeta = exp(k/N) - 1;                  // upsampling rate  // eq. 16
    int H_X = H_X0;

/// At this point, X has been input and arrays have been set up.
/// Perform a sweep of R values

    FILE *imgfp;
    if (verbose) {
        imgfp = fopen("image.csv", "w+");       // save image to file
    }

    printf("N=%d, H_X=%d, k=%f, zeta=%f\n", N, H_X0, k, zeta);

	printf("Processing %d R values into %dx%d image\n", Rsteps, 2*IMG_H, IMG_H);
	for (int step=0; step<Rsteps; step++) {
        float R = MinR + (float)step * (MaxR-MinR) / (float)(Rsteps-1);
		// M is the number of X input samples to warp to Y. Usually N to 4N.
		float M = -N * log(1 - N*(1 - exp(-fabs(R)/N))) / fabs(R);  // eq. 7
		// rate constant for downsampling exponential sweep
		float lambda = exp(-R/N) - 1;                               // eq. 9
		// real H_V, ideal offset in V samples
		float H_V = (float)H_X0 * fabs(R) / k;

		float pitch = 1.0;                                          // eq. 10
		if (R>0) {
			pitch = exp(M*R/N);
		}

        for (int i=0; i<(N/2); i++) {           // clear V, use 1.0 so log is 0
            V[i] = 1;
        }

// The number of Vout points painted to the arc should be limited to twice the
// arc length in pixels. When R=0.8, the limit is IMG_H*pi.

        #ifdef RADIAL
        int MaxVpoints = (int)(R * IMG_H * 2.0 * PI / 0.8);
        #else
        int MaxVpoints = 2*IMG_H;
        #endif // RADIAL

        uint32_t xoffset = 0;
        float voffset = 0;
        uint32_t VoutSize = 0;                  // points in the output

        if (verbose) {
            printf("M=%.2f, R=%.4f, pitch=%.4f, lambda=%.6f, H_V=%.4f\n",M,R,pitch,lambda,H_V);
        } else {
            printf(".");
        }
        int ivoffset = 0;
        while ((xoffset < (Xlength-(int)H_X0)) && (VoutSize < MaxVpoints)) {
			compress(&X[xoffset], XW, N, pitch, lambda, -lambda/2, 0);
			for (int i=0; i<N; i++) {           // copy real to complex
				Y[i].r = XW[i];  Y[i].i = 0;
			}
			HannWindow(Y);
			kiss_fft(cfg, Y, U);
			for (int i=0; i<(N/2); i++) {
				mag2[(N/2-1)-i] = U[i].r * U[i].r + U[i].i * U[i].i;
			}
			memset(W, 0, (N/2)*sizeof(float));  // clear W
			float nextVoffset = voffset + H_V;
			int vWidth = (int)nextVoffset - (int)voffset;
			voffset = nextVoffset;
			int Widxlast = N/2 - vWidth;
			compress(mag2, W, Widxlast, 1, -zeta, 0, 1);
			// Correlate Ws in V using an offset
			int Rsign = 0;
			if (R<0) { Rsign = -1; }
			for (int i=0; i<Widxlast; i++) {    // correlate
				if (Rsign) {                    // R < 0
					V[vmask & (ivoffset + i)] += W[(Widxlast-1) - i];
				} else {                        // R > 0
					V[vmask & (ivoffset + i)] += W[i];
				}
			}
        // Note: The first N/2 output points are weaker than the rest
			for (int i=0; i<vWidth; i++) {     // output finished points
                float dB = 10 * log10( V[vmask & (i + ivoffset)] );
                /* clear after use */  V[vmask & (i + ivoffset)] = 1;
                Vout[VoutSize++] = dB;
			}
			xoffset += H_X;
			ivoffset += vWidth;
        }
        int MinVpoints = (H_V*(N/2))/H_X;
        #ifdef RADIAL
        int radius = (int)(R * 65536.0 / 0.8);  // scale radius to 16-bit
        float anglescale = 32768.0 / MaxVpoints;
        if (MinVpoints < MaxVpoints) {
            for (int i=MinVpoints; i<=MaxVpoints; i++) {
                PlotPixel(Vout[i], radius, anglescale*(float)i);
            }
        }
        #else
        if (MinVpoints < MaxVpoints) {
            for (int i=MinVpoints; i<MaxVpoints; i++) { // column sweep
                XYpixel(Vout[i], i, step);      // rectangular format
            }
        }
        #endif
        if (verbose) {
            for (int i=0; i<MaxVpoints; i++) {
                fprintf(imgfp, "%g", Vout[i]);
                if (i==MaxVpoints-1) {
                    fprintf(imgfp, "\n");
                } else {
                    fprintf(imgfp, ",");
                }
            }
        }
	}
    if (verbose) {
        fclose(imgfp);
    }
    printf("\n");
//	TestPattern();

/// Convert image to BMP
	float stats[3];
    ImageStats(stats);
    if (autoCeil)  {ceilColor  = stats[1] - 4;}
    if (autoFloor) {floorColor = stats[0] - 4;}

	printf("Saving BMP to %s\n", outfilename);
	printf("Average=%g, Max=%g; Heatmap: Ceiling=%g, Floor=%g\n",
        stats[0], stats[1], ceilColor, floorColor);
	SaveImage(outfilename);

/// Finished
    BMPfree();
    kiss_fft_free(cfg);
    FreeHann();
    free(Vout);
    free(V);
    free(W);
    free(mag2);
    free(U);
    free(Y);
    free(XW);
    free(X);
    printf("Total execution time %.3g sec.\n", 1e-6*(now() - startTime));
    return 0;
}
