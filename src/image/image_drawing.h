#pragma once

#include "image.h"
#include "simulation/simulation.h"

void image_draw_circle(Image* i, int cx, int cy, int r, RGB8 color);
void image_draw_line(Image* i, int x0, int y0, int x1, int y1, RGB8 c);
void image_draw_vector(Image* i, int x0, int y0, int x1, int y1, RGB8 c);
void image_draw_square_alligned(Image* img, int x, int y, int size_x, int size_y, RGB8 c);
