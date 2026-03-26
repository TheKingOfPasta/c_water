#pragma once

#include "image.h"
#include "particule.h"

#define NB_PARTICULES 200

typedef struct
{
    int sx;
    int sy;

    Particule particules[NB_PARTICULES];

    // 2d array of size sx * sy
    float* density_field;
} Simulation;

Simulation simulation_gen(int sx, int sy);
void simulation_free(Simulation* s);

void simulation_step(Simulation* s);

Vec2 simulation_compute_gradient(Simulation* s, int x, int y);

void simulation_draw_balls(Simulation* s, Image* img, int padding);
void simulation_draw_field(Simulation* s, Image* img, int padding);
void simulation_draw_field_arrow(Simulation* s, Image* img, int padding);
void simulation_draw_mouse_gradient(Simulation* s, Image* img, int padding,
                                    int mouse_x, int mouse_y);
