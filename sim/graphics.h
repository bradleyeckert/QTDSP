#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#define PI 3.1415926538

int BMPalloc(char *filename);   // call before using
void BMPfree(void);             // call after finished

void PlotPixel(float z, uint16_t rho, uint16_t theta);
void XYpixel(float z, int x, int y);
void SaveImage(char *filename, int offset, int outputRate);
void ImageStats(float* stats);
void SetHeatMapScheme (uint8_t* map);

extern float floorColor;
extern float ceilColor;
extern int IMG_W;
extern int IMG_H;

#endif

