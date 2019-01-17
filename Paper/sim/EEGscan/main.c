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
#include <stdint.h>
#include <math.h>
#define PI 3.1415926538
#define IMG_H 160 // 780
#define IMG_W (2*IMG_H)	// must be a multiple of 8
#define imageBytes (sizeof(float) * IMG_W * IMG_H)

float *image;

void SetXYpixel(float z, int x, int y) {
    image[ y*IMG_W + x ] += z;
}

// rho is 0 to 65535
// theta is 0 to PI

void PlotPixel(float z, float rho, float theta) {
    float x,y;
    x = rho * cos(theta) * (float)IMG_H + (IMG_H<<16);
    y = rho * sin(theta) * (float)IMG_H;
    uint32_t ix = round(x);  uint16_t fx = ix;
    uint32_t iy = round(y);  uint16_t fy = iy;
    ix = ix>>16;
    iy = iy>>16;
    if (fx&0x8000) {
        if (fy&0x8000) {    // SE
        } else {            // NE
        }
    } else {
        if (fy&0x8000) {    // SW
        } else {            // NW
        }
    }
    SetXYpixel(z,ix,iy);
}

void LoadImage(void) {
	// fill image with a test pattern
/*	for(int i=0; i<IMG_H; i++)
	{
	    for(int j=0; j<IMG_W; j++)
		{
		    image[i*IMG_W+j] = 127.0 * (1.0 - cos((float)(i+j)/30));
		}
	}
*/
	    for(int j=0; j<314; j++)
		{
		    PlotPixel(127, 40000, (float)j/100);
		}
}

/* Save the 2D floating point image in BMP format with (0,0) at upper left.
*/

void SaveImage(void) {
	FILE *f;
	int filesize = 54 + 3*IMG_W*IMG_H;  // BMP boilerplate plus image
	uint8_t pixel[3];					// B G R
	uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};

	bmpfileheader[ 2] = (uint8_t)(filesize    );
	bmpfileheader[ 3] = (uint8_t)(filesize>> 8);
	bmpfileheader[ 4] = (uint8_t)(filesize>>16);
	bmpfileheader[ 5] = (uint8_t)(filesize>>24);
	bmpinfoheader[ 4] = (uint8_t)(IMG_W    );
	bmpinfoheader[ 5] = (uint8_t)(IMG_W>> 8);
	bmpinfoheader[ 6] = (uint8_t)(IMG_W>>16);
	bmpinfoheader[ 7] = (uint8_t)(IMG_W>>24);
	bmpinfoheader[ 8] = (uint8_t)(IMG_H    );
	bmpinfoheader[ 9] = (uint8_t)(IMG_H>> 8);
	bmpinfoheader[10] = (uint8_t)(IMG_H>>16);
	bmpinfoheader[11] = (uint8_t)(IMG_H>>24);

	f = fopen("img.bmp","wb");
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	for(int i=0; i<IMG_H; i++)
	{
		for(int j=0; j<IMG_W; j++)
		{
		    int z = image[(IMG_H-i-1)*IMG_W+j];
		    if (z<0)   {z=0;}
		    if (z>255) {z=255;}
			pixel[0] = z;
			pixel[1] = z;
			pixel[2] = z;
			fwrite(pixel,1,3,f);
		}
	}
	fclose(f);
}

///////////////////////////////////////////////////////////////////////////////
int main()
{
	image = malloc(imageBytes);         // numeric image
	if (image==0) {
        return 1;                       // memory error
	}
	memset(image,0,imageBytes);
	LoadImage();
	SaveImage();
	free(image);

    return 0;
}
