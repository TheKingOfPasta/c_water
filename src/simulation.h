#pragma once

#include "image.h"
#include "vec2.h"

#define NB_POINTS 100

typedef struct
{
    int sx;
    int sy;

    Vec2 points[NB_POINTS];
} Simulation;

Simulation simulation_gen(int sx, int sy);

void simulation_draw(Simulation* s, Image* img);
