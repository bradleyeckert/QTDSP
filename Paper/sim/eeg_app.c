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

void LoadImage(void) {
	// fill image with a test pattern
    for(int i=200; i<1000; i++) {
    for(int j=0; j<(2*i); j++) {
        PlotPixel(200, i<<6, (float)j*16380.0/(float)i);
    }}
}

float* X;                                       // X array
int Xlength = 0;                                // points in X
char header[128];

float Number(char* str) {	// convert string to floating point number
	float d;
	sscanf(str, "%f", &d);
	return d;
}

/**
**/
int main(int argc, char *argv[])
{
    char *outfilename = "img.bmp";
    char *infilename = "excerpt2.txt";
    int m = 3;                                  // decimation factor
    float MaxR = 0.78;
    float MinR = 0.20;

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
				case 'r':	// set new Rmin
					MinR = Number(argv[Arg++]); break;
				case 'R':	// set new Rmax
					MaxR = Number(argv[Arg++]); break;
				case 'f':	// set floor of heatmap
					floorColor = Number(argv[Arg++]); break;
				case 'c':	// set ceiling of heatmap
					ceilColor = Number(argv[Arg++]); break;
				default: break;
			}
		}
	}

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

    fscanf(ifp, "%s", header);                  // 1st line is header string
    int j = m;
    while (Xlength<MAXPOINTS) {
        float x, xf;
        if (1!=fscanf(ifp, "%f", &x)) break;    // floating point numbers
        xf = BiQuad(x, &LowPass);               // get filtered version
        if (j==1) {
            X[Xlength++] = xf;
            j = m;
        } else {
            j--;
        }
    }
    fclose(ifp);
	printf("%d points of %s data\n", Xlength*m, header);
	printf("Decimation = %d, X = %d points, R = %.3f to %.3f\n",
        m, Xlength, MinR, MaxR);

///
	printf("Creating Image\n");
	LoadImage();
	printf("Saving BMP to %s\n", outfilename); fflush(stdout);
	SaveImage(outfilename);

/// Output decimated X to "decimated.txt"
	FILE *ofp;
	ofp = fopen("decimated.txt", "w");
    if (ofp != NULL) {
        for(int i=0; i<Xlength; i++) {
            fprintf(ofp, "%g\n", X[i]);
        }
    }

    BMPfree();
    free(X);
    printf("Total execution time %.3g sec.\n", 1e-6*(now() - startTime));
    return 0;
}
