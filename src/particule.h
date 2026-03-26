#pragma once

#include "vec2.h"

#define PARTICULE_RADIUS 70

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particule;

Particule particule_gen_random(int sx, int sy);

void particule_step(Particule* p, int sx, int sy);

void particule_print(Particule* p);

float particule_density(Particule* p, Vec2 sample);
