#pragma once

#include "image.h"
#include "vec2.h"

#define NB_PARTICULES 100

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particule;

typedef struct
{
    int sx;
    int sy;

    Particule particules[NB_PARTICULES];
} Simulation;

Simulation simulation_gen(int sx, int sy);
void simulation_step(Simulation* s);

void simulation_draw(Simulation* s, Image* img);
