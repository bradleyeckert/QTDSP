/*******************************************************************************
Tools for generating a 24-bit BMP, writing it to a file, and setting pixels
in polar format.

(c)2019 Brad Eckert. Revision 0.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#define imageBytes (sizeof(float) * IMG_W * IMG_H)
#define SCALE16 (1.0/65536.0)
#define NUM_COLORS 5

float *image;
float weight[32][32];                   // weight table for pixel smoothing
float *sintable;                        // 1st quadrant of sine

static float sin16(int angle) {         // sine from 16-bit angle
    int ang = angle & 0xFFFF;
    if (angle & 0x4000) {
        ang = ~angle;
    }
    float x = sintable[ang & 0x3FFF];
    if (angle & 0x8000) {
        x = -x;
    }
    return x;
}
static float cos16(int angle) {         // cosine from 16-bit angle
    return sin16(angle + 0x4000);
}

/// Initialize BMP
int BMPalloc(void) {
	image = malloc(imageBytes);         // numeric image
	sintable = malloc(sizeof(float) * 0x4002);
	if (!image) return 1;               // memory error
	if (!sintable) return 1;
	memset(image,0,imageBytes);
	for(int i=0; i<32; i++) {           // set up weight table
	for(int j=0; j<32; j++) {
        weight[i][j] = sqrt(i*i + j*j)/32;
	}}
	for(int i=0; i<0x4002; i++) {       // set up sine table
        sintable[i] = sin((float)i*PI/0x8000);
	}
	return 0;
}

/// Free BMP
void BMPfree(void) {
    free(sintable);
    free(image);
}

static void SetXYpixel(uint16_t fx, uint16_t fy, float z, int x, int y) {
    if ((x<0) || (y<0)) return;
    image[ y*IMG_W + x ] += z * weight[fx>>11][fy>>11];
}

// x and y coordinates are 2^16 * pixel position
static void SmoothPixel(float z, float x, float y) {
    uint16_t x0,y0,x1,y1;
    uint32_t ix = round(x);  uint16_t fx = ix;
    uint32_t iy = round(y);  uint16_t fy = iy;
    ix = ix>>16;
    iy = iy>>16;
    if (fx&0x8000) {
        if (fy&0x8000) {                        // NW_________________________
            x0 = fx ^ 0x7FFF;    				// 8000 to FFFF = 1 to 1/2
            y0 = fy ^ 0x7FFF;    				// 8000 to FFFF = 1 to 1/2
            x1 = fx & 0x7FFF;    				// 8000 to FFFF = 1/2 to 0
            y1 = fy & 0x7FFF;    				// 8000 to FFFF = 1/2 to 0
            SetXYpixel(x0, y0, z, ix,   iy);    // 00
            SetXYpixel(x1, y0, z, ix+1, iy);    // 10
            SetXYpixel(x1, y1, z, ix+1, iy+1);  // 11
            SetXYpixel(x0, y1, z, ix,   iy+1);  // 01
        } else {                                // SW_________________________
            x0 = fx ^ 0x7FFF;    				// 8000 to FFFF = 1 to 1/2
            y1 = fy | 0x8000;    				// 0000 to 7FFF = 1 to 1/2
            x1 = fx ^ 0xFFFF;    				// 8000 to FFFF = 1/2 to 0
            y0 = fy ^ 0x7FFF;    				// 0000 to 7FFF = 1/2 to 0
            SetXYpixel(x0, y1, z, ix,   iy);    // 01
            SetXYpixel(x1, y1, z, ix+1, iy);    // 11
            SetXYpixel(x1, y0, z, ix+1, iy-1);  // 10
            SetXYpixel(x0, y0, z, ix,   iy-1);  // 00
        }
    } else {
        if (fy&0x8000) {                        // NE_________________________
            x1 = fx | 0x8000;    				// 0000 to 7FFF = 1 to 1/2
            y0 = fy ^ 0x7FFF;    				// 8000 to FFFF = 1 to 1/2
            y1 = fy ^ 0xFFFF;    				// 8000 to FFFF = 1/2 to 0
            x0 = fx ^ 0x7FFF;    				// 0000 to 7FFF = 1/2 to 0
            SetXYpixel(x1, y0, z, ix,   iy);    // 10
            SetXYpixel(x1, y1, z, ix,   iy+1);  // 11
            SetXYpixel(x0, y1, z, ix-1, iy+1);  // 01
            SetXYpixel(x0, y0, z, ix-1, iy);    // 00
        } else {                                // SE
            x1 = fx ^ 0xFFFF;    				// 0000 to 7FFF = 1 to 1/2
            y1 = fy ^ 0xFFFF;    				// 0000 to 7FFF = 1 to 1/2
            x0 = fx ^ 0x7FFF;    				// 0000 to 7FFF = 1/2 to 0
            y0 = fy ^ 0x7FFF;    				// 0000 to 7FFF = 1/2 to 0
            SetXYpixel(x1, y1, z, ix,   iy);    // 11
            SetXYpixel(x0, y1, z, ix-1, iy);    // 01
            SetXYpixel(x0, y0, z, ix-1, iy-1);  // 00
            SetXYpixel(x1, y0, z, ix,   iy-1);  // 10
        }
    }
}

/// Plot in polar format with zero theta in the NW corner.
/// rho is 0 to 65535
/// theta is 0 to 32767
void PlotPixel(float z, uint16_t rho, uint16_t theta) {
    theta &= 0x7FFF;
    float x = (float)rho * cos16(theta) * -(float)IMG_H + (IMG_H<<16);
    float y = (float)rho * sin16(theta) * (float)IMG_H + 1.0;
    SmoothPixel(z,x,y);
}

/** Save the 2D floating point image in BMP format with (0,0) at upper left.
A 7-color heat map is used to create 6 color gradients between floor and ceil.
*/

 float floorColor = 0;
 float ceilColor = 6000;

// see http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
// modified to paint 24-bit pixels in a BMP, use 7 colors, and compile as C.
void getHeatMapColor(float z, uint8_t *bgr)
{
    static float color[NUM_COLORS][3] = {
//        {  0,   0,   0}, // black
        {  0,   0, 255}, // blue
        {  0, 255, 255}, // cyan
        {  0, 255,   0}, // green
        {255, 255,   0}, // yellow
        {255,   0,   0}, // red
//        {255, 255, 255}  // white
    };
    float value = (z - floorColor) / (ceilColor - floorColor); // scale to 0-1
    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    float span = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0)        { idx1 = idx2 = 0;                // accounts for an input <=0
    } else if(value >= 1) { idx1 = idx2 = NUM_COLORS-1;     // accounts for an input >=0
    } else {
        value = value * (NUM_COLORS-1); // Will multiply value by 3.
        idx1  = floor(value);           // Our desired color will be after this index.
        idx2  = idx1+1;                 // ... and before this index (inclusive).
        span = value - (float)idx1;     // Distance between the two indexes (0-1).
    }
    bgr[2] = (uint8_t)((color[idx2][0] - color[idx1][0])*span + color[idx1][0]);
    bgr[1] = (uint8_t)((color[idx2][1] - color[idx1][1])*span + color[idx1][1]);
    bgr[0] = (uint8_t)((color[idx2][2] - color[idx1][2])*span + color[idx1][2]);
}

void SaveImage(char *filename) {
	FILE *f;
	int filesize = 54 + 3*IMG_W*IMG_H;  // BMP boilerplate plus image
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

	f = fopen(filename,"wb");
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	uint8_t *line = (uint8_t *)malloc(IMG_W*3);
	for(int i=0; i<IMG_H; i++)
	{
		for(int j=0; j<IMG_W; j++)
		{
		    int z = image[(IMG_H-i-1)*IMG_W+j];
		    getHeatMapColor(z, &line[j*3]);
/*		    if (z<0)   {z=0;}
		    if (z>255) {z=255;}
		    line[j*3+0] = z;
		    line[j*3+1] = z;
		    line[j*3+2] = z; */
		}
        fwrite(line,3,IMG_W,f);
	}
	fclose(f);
	free(line);
}
