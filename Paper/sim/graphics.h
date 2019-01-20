#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#define PI 3.1415926538
#define IMG_W (2*IMG_H)	// must be a multiple of 8

int BMPalloc(void);     // call before using
void BMPfree(void);     // call after finished

void PlotPixel(float z, uint16_t rho, uint16_t theta);
void SaveImage(char *filename);
void LogImage(void);
void ImageStats(float* stats);

extern float floorColor;
extern float ceilColor;
extern int IMG_H;

#endif

