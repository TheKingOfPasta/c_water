#pragma once

#include "vec2.h"

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particule;

Particule particule_gen_random(int sx, int sy);

void particule_step(Particule* p, int sx, int sy);

void particule_print(Particule* p);
