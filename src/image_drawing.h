#pragma once

#include "image.h"

void image_draw_circle(Image* i, int cx, int cy, int r, RGB8 color);
void image_draw_line(Image* i, int x0, int y0, int x1, int y1, RGB8 c);
void image_draw_vector(Image* i, int x0, int y0, int x1, int y1, RGB8 c);
