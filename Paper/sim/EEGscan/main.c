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

void LoadImage(void) {
	// fill image with a test pattern
    for(int i=200; i<1000; i++) {
    for(int j=0; j<(2*i); j++) {
        PlotPixel(200, i<<6, (float)j*16380.0/(float)i);
    }}
}

///////////////////////////////////////////////////////////////////////////////
int main()
{
    char *filename = "img.bmp";
	printf("Initializing BMP\n"); fflush(stdout);
	if (BMPalloc()) {
        return 1;                       // memory error
	}
	printf("Creating Image\n"); fflush(stdout);
	LoadImage();
	printf("Saving BMP to %s\n", filename); fflush(stdout);
	SaveImage(filename);
    BMPfree();
    return 0;
}
