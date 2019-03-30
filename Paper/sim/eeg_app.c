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
#define MINR (Rsign*exp(MinR))
#define MAXR (Rsign*exp(MaxR))
#define PosSlope 10

char header[128];

float Number(char* str) {	// convert string to floating point number
	float d;
	sscanf(str, "%f", &d);
	return d;
}

/**
**/
char outfilename[256] = "img.bmp";
char infilename[256] = "eeg.txt";

static uint8_t mono_color[2*3+1] = {    // monochrome (light) RGB color palette
    2, // numcolors R G B R G B ...
	0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00};

int main(int argc, char *argv[])
{
    int m = 3;              // decimation factor
    int N = 1024;
    int H_X0 = 16;
    float MaxR = -0.20;
    float MinR = -0.78;
    float gamma = 1.0;
    int samplerate = 250;
    int offset = 0;         // offset of starting point in file
    char autoCeil = 1;
    char autoWidth = 0;
    char verbose = 0;
    int floorSpan = 20;     // dB below ceiling to set floor, if nonzero

	float * X;              // raw input
	float * XW;             // warped input
    kiss_fft_cpx * Y;       // FFT input (copy of XW)
    kiss_fft_cpx * U;       // FFT result
	float * mag2;           // magnitude^2 of FFT result
	float * W;              // warped mag^2
	float * V;              // correlation buffer
	float * Vout;           // output
	float * Row;            // fit-to-row output
	float * Rpeaks;         // peak and total power for a given R

/// Overwrite defaults using command line arguments
	int Arg = 1;
	while (argc>Arg) {		// parse single-character arguments
		if (strlen(argv[Arg]) == 1) {
            char cmdchar = argv[Arg++][0];
			switch(cmdchar) {
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
					if (N>16384 || N<128) {
                        fprintf(stderr, "N range is 128 to 16384\n");
                        return 4;
					}
					break;
				case 'r':	// set new Rmin
					MinR = Number(argv[Arg++]); break;
				case 'R':	// set new Rmax
					MaxR = Number(argv[Arg++]); break;
				case 'g':	// set new gamma
					gamma = Number(argv[Arg++]);
					if (gamma<0.1 || gamma>1.0) {
                        fprintf(stderr, "g: Allowed gamma 0.1 to 1.0\n");
                        return 4;
					} break;
				case 'x':	// set new X step size
					H_X0 = (int)Number(argv[Arg++]);
					if (H_X0<4 || H_X0>100) {
                        fprintf(stderr, "v: H_X range is 4 to 100\n");
                        return 4;
					} break;
				case 'f':	// set floor of heatmap
					floorColor = Number(argv[Arg++]);	break;
				case 'c':	// set ceiling of heatmap
					ceilColor = Number(argv[Arg++]);
                    autoCeil = 0;	break;
                case 'h':   // set BMP height to multiple of 4
                    IMG_H = (3 + (int)Number(argv[Arg++])) & 0xFFFC;  break;
                case 'w':   // set BMP width
                    IMG_W = (int)Number(argv[Arg++]);  autoWidth = 0;  break;
                case 'v':   verbose = 1;  break;
                case 'a':   autoWidth = 1;  break;
                case 'F':   // set floor depth relative to ceiling in dB
                    floorSpan = (int)Number(argv[Arg++]);  break;
                case 'M':   SetHeatMapScheme(mono_color);  break;
                case 'z':   comp_filtered = 1;  break;
				default: printf("Unknown command %c\n", cmdchar);  break;
			}
		}
	}
	int Rsign = 1;
    if (fabs(MinR) > 0.78 || fabs(MaxR) > 0.78) {
        fprintf(stderr, "Max |R| is 0.78\n");  return 4; }
    if ((MinR*MaxR)<0) {
        fprintf(stderr, "Min R and Min R must be of the same sign\n");
        return 4;      }
    if (fabs(MinR) > fabs(MaxR)) {  // if the R range is backward, fix it
        float temp = MinR;  MinR = MaxR;  MaxR = temp;
    }
    if (MaxR < 0) {
        Rsign = -1;
    }
    MinR = log(fabs(MinR));
    MaxR = log(fabs(MaxR));

    double startTime = now();
	X = malloc(sizeof(float) * MAXPOINTS);
    memset(X, 0, sizeof(float) * MAXPOINTS);

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
	printf("Decimation = %d, X = %d points, R = %.3f to %.3f, Fs=%f\n",
        m, Xlength, MINR, MAXR, (float)samplerate/m);

/// Output decimated X to "decimated.txt" for sanity checking
	FILE *ofp;
	if (verbose) {
        ofp = fopen("decimated.txt", "w");
        if (ofp != NULL) {
            for(uint32_t i=0; i<Xlength; i++) {
                fprintf(ofp, "%g\n", X[i]);
            }
        }
        fclose(ofp);
	}

    if (autoWidth) {
    IMG_W = (3 + (Xlength / 6)) & 0xFFFFFFFCL;  // fit BMP width to data
    }

	if (BMPalloc("colors.txt") || (X==0)) { return 1; }  // memory error

/// So far, memory allocation has gone okay. The following bunch is not checked.

    int nbytes = N * sizeof(kiss_fft_cpx);
    XW=(float *)malloc(N * sizeof(float));
    Y=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    U=(kiss_fft_cpx*)KISS_FFT_MALLOC(nbytes);
    mag2=(float *)malloc(N * sizeof(float)/2);
    W=(float *)malloc(N/2 * sizeof(float));
    V=(float *)malloc(N/2 * sizeof(float));
    Vout=(float *)malloc(MAXPOINTS * sizeof(float));
    Row=(float *)malloc(IMG_W * sizeof(float));
    Rpeaks=(float *)malloc(3*IMG_H * sizeof(float));
    memset(Rpeaks, 0, 3*IMG_H*sizeof(float));

    kiss_fft_cfg cfg = kiss_fft_alloc(N,0,0,0 );
    CreateHann(N);                              // set up the FFT window
	int vmask = N/2 - 1;						// one less than an exact power of 2

    // k is an upsampling constant, about 2
    double k = N * log(((double)N-2)/((double)N-(2+2/gamma)));      // eq. 15
    int maxIdx = (N/2)*gamma;
    double zeta = exp(k/N) - 1;         		// upsampling rate  // eq. 16
    int H_X = H_X0;
    double pixScale = H_X / 5;                  // scale to Fs/5 output rate
    double outputRate = samplerate * pixScale / (H_X*m); // pixels per second

    int firstOutput = offset / (m * 5);         // output points discarded before left edge

/// At this point, X has been input and arrays have been set up.
/// Perform a sweep of R values

    FILE *imgfp;
    if (verbose) {
        imgfp = fopen("image.csv", "w+");       // save image to file
    }

    printf("N=%d, H_X=%d, k=%f, zeta=%f, Out=%.2f pels/sec\n", N, H_X0, k, zeta, outputRate);
    printf("maxIdx=%d\n", maxIdx);

	printf("Processing %d R values into %d x %d image\n", IMG_H, IMG_W, IMG_H);
	for (int step=0; step<IMG_H; step++) {
        double R = exp(MinR + (double)step * (MaxR-MinR) / (double)(IMG_H-1)) * Rsign;
/// M is the number of X input samples to warp to Y. Usually N to 4N.
		double M = -N * log(1 - N*(1 - exp(-fabs(R)/N))) / fabs(R); // eq. 7
/// rate constant for downsampling exponential sweep
		double lambda = exp(-R/N) - 1;                              // eq. 9
/// real H_V, ideal offset in V samples
		double H_V = (double)H_X * fabs(R) / k;
        double RowScale = H_V / pixScale;
/// slope correction for R>0
	    int SlopeFix = 0;
	    if (R>0) SlopeFix = M * R / exp(R);

		float pitch = 1.0;                                          // eq. 10
		if (R>0) {
			pitch = exp(M*R/N);
		}

        for (int i=0; i<(N/2); i++) {          // clear V, use 1.0 so log is 0
            V[i] = 1;
        }

        int MaxVpoints = IMG_W * RowScale;
        uint32_t xoffset = 0;
        double voffset = 0;
        uint32_t VoutSize = 0;                  // points in the output

        if (verbose) {
            printf("M=%.2f, R=%.4f, pitch=%.4f, lambda=%.6f, H_V=%.4f\n",M,R,pitch,lambda,H_V);
        } else {
            printf(".");
        }
        int ivoffset = 0;
        while (VoutSize < MaxVpoints) {
/// downsample
			compress(&X[xoffset], XW, N, pitch, lambda, lambda, pitch, 0);
			for (int i=0; i<N; i++) {           // copy real to complex
				Y[i].r = XW[i];  Y[i].i = 0;
			}
/// FFT
			HannWindow(Y);
			kiss_fft(cfg, Y, U);
			for (int i=0; i<maxIdx; i++) {
				mag2[(maxIdx-1)-i] = U[i].r * U[i].r + U[i].i * U[i].i;
			}
/// upsample
			memset(W, 0, (N/2)*sizeof(float));  // clear W
			double nextVoffset = voffset + H_V;
			int vWidth = (int)nextVoffset - (int)voffset;
			voffset = nextVoffset;
			int Widxlast = maxIdx-1;
			compress(mag2, W, Widxlast, 1, -zeta, 0, 1, 1);
/// Correlate Ws in V using an offset
			for (int i=0; i<Widxlast; i++) {    // correlate
				if (R<0) {                      // R < 0
					V[vmask & (ivoffset + i)] += W[(Widxlast-1) - i];
				} else {                        // R > 0
					V[vmask & (ivoffset + i)] += W[i];
				}
			}
/// Note: The first maxIdx output points are weaker than the rest
/// Process the outputs
			for (int i=0; i<vWidth; i++) {      // output finished points
                float dB = 10 * log10( V[vmask & (i + ivoffset)] / fabs(R));
                /* clear after use */  V[vmask & (i + ivoffset)] = 1;
                Vout[VoutSize++] = dB;
                if (dB > Rpeaks[step*3]) {      // track the peaks
                    Rpeaks[step*3] = dB;
                }
                Rpeaks[step*3+1] += exp(dB/10); // accumulate power for RMS
			}
			xoffset += H_X;
			ivoffset += vWidth;
        }
/// stretch the row of pixels to fit the normalized width
        compress(Vout, Row, IMG_W, RowScale, 0, 0, 1/RowScale, 1);

        for (int i=0; i<IMG_W; i++) {           // column sweep
            XYpixel(Row[i], i+SlopeFix, step);  // rectangular format
        }
        if (verbose) {                          // v saves CSV file
            for (int i=0; i<IMG_W; i++) {
                fprintf(imgfp, "%g", Row[i]);
                if (i==IMG_W-1) {
                    fprintf(imgfp, "\n");
                } else {
                    fprintf(imgfp, ",");
                }
            }
        }
        Rpeaks[step*3+2] = R;
        Rpeaks[step*3+1] /= fabs(R);
	}
    if (verbose) {
        fclose(imgfp);
    }

    printf("\n");

/// Convert image to BMP
	float stats[3];
    ImageStats(stats);
    if (autoCeil) {     // automatic ceiling level
        ceilColor  = stats[1] - 1;
    }
    if (floorSpan) {    // fixed span
        floorColor = ceilColor - floorSpan;
    } else {            // automatic span
        floorColor = stats[0] - 1;
    }

	printf("Saving BMP to %s\n", outfilename);
	printf("Average=%g, Max=%g; Heatmap: Ceiling=%g, Floor=%g\n",
        stats[0], stats[1], ceilColor, floorColor);
	SaveImage(outfilename, firstOutput, outputRate);

/// Save the R peaks to a file by changing the extension of the BMP filename
    int len = strlen(outfilename);
    memcpy(&outfilename[len-3], "csv", 3);
    FILE *rfp;
    rfp = fopen(outfilename, "w");
    if (rfp) {
        printf("Writing peaks to %s\n", outfilename);
        for (int i=0; i<IMG_H; i++) {
            fprintf(rfp, "%g,%g,%g\n", Rpeaks[i*3+2], Rpeaks[i*3], Rpeaks[i*3+1]);
        }
        fclose(rfp);
    }

/// Finished
    BMPfree();
    kiss_fft_free(cfg);
    FreeHann();
/// Free memory. Freeing in reverse order is not required, I just like it.
    free(Rpeaks);
    free(Row);
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
