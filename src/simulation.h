#pragma once

#include "image.h"
#include "vec2.h"

#define PRESSURE_FORCE 2000
#define TARGET_PRESSURE 0.2
#define PARTICULE_INFLUENCE_RADIUS 20 // for density
#define PARTICULE_RADIUS 6 // for collision & drawing
#define GRAVITY_MULTIPLIER 4.0
#define VELOCITY_COLLISION_DAMPNER 1.0 // 1.0 no loss - 0.0 100% loss
#define DRAG 0.5

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particule;

#define NB_PARTICULES 2000

typedef struct
{
    int sx;
    int sy;

    Particule particules[NB_PARTICULES];

    float density_field[NB_PARTICULES];
} Simulation;

/* --- PARTICLES --- */
Particule particule_gen_random(int sx, int sy);

void particule_step(Simulation* s, Particule* p);

void particule_print(Particule* p);

float particule_density(float d);
Vec2 particule_compute_gradient(Simulation* s, Particule* p);

/* --- SIMULATION --- */

Simulation simulation_gen(int sx, int sy);
void simulation_free(Simulation* s);

void simulation_step(Simulation* s);

float simulation_compute_density(Simulation* s, Particule* p);

void simulation_draw_balls(Simulation* s, Image* img);
void simulation_draw_field(Simulation* s, Image* img);
