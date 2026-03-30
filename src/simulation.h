#pragma once

#include "image.h"
#include "vec2.h"

#define PRESSURE_FORCE 10
#define TARGET_PRESSURE 3
#define PARTICULE_RADIUS 50

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particule;

#define NB_PARTICULES 2

typedef struct
{
    int sx;
    int sy;

    Particule particules[NB_PARTICULES];

    // 2d array of size sx * sy
    float* density_field;
} Simulation;

/* --- PARTICLES --- */
Particule particule_gen_random(int sx, int sy);

void particule_step(Simulation* s, Particule* p);

void particule_print(Particule* p);

float particule_density(Particule* p, Vec2 sample);

/* --- SIMULATION --- */

Simulation simulation_gen(int sx, int sy);
void simulation_free(Simulation* s);

void simulation_step(Simulation* s);

Vec2 simulation_compute_gradient(Simulation* s, float x, float y);

void simulation_draw_balls(Simulation* s, Image* img);
void simulation_draw_field(Simulation* s, Image* img);
void simulation_draw_field_arrow(Simulation* s, Image* img);
void simulation_draw_mouse_gradient(Simulation* s, Image* img,
                                    int mouse_x, int mouse_y);
