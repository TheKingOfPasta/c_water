#pragma once

#include "colorRGB8.h"

#define IMAGE_NB_LEVELS 256

#define TL_IMAGE_ALIGNMENT 64

typedef struct Image
{
    int sx;
    int sy;

    RGB8* pixels;
} Image;

Image image_blank(int sx, int sy);

void image_set_color(Image* i, int x, int y, RGB8 c);

void image_fill(Image* i, RGB8* c);

void image_savePPM(const Image* i, const char* filename);

bool image_in_bounds(Image* i, int x, int y);
