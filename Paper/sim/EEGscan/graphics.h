#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#define PI 3.1415926538
#define IMG_H 400
#define IMG_W (2*IMG_H)	// must be a multiple of 8

int BMPalloc(void);
void BMPfree(void);
void PlotPixel(float z, uint16_t rho, uint16_t theta);
void SaveImage(char *filename);
extern float floorColor;
extern float ceilColor;

#endif

