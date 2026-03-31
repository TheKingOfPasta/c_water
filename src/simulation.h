#pragma once

#include "image.h"
#include "vec2.h"

#define VELOCITY_COLLISION_DAMPNER 0.9 // 1.0 no loss - 0.0 100% loss
#define DRAG 0.995

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

    float pressure_force;
    float target_pressure;
    float particule_influence_radius;
    float radius;
    float gravity_multiplier;

    Particule particules[NB_PARTICULES];

    float density_field[NB_PARTICULES];
} Simulation;

/* --- PARTICLES --- */
Particule particule_gen_random(int sx, int sy);

void particule_step(Simulation* s, Particule* p);

void particule_print(Particule* p);

float particule_density(float d, float influence_radius);
Vec2 particule_compute_gradient(Simulation* s, Particule* p);

/* --- SIMULATION --- */

Simulation simulation_gen(int sx, int sy);
void simulation_free(Simulation* s);

void simulation_step(Simulation* s);

float simulation_compute_density(Simulation* s, Particule* p);

void simulation_draw_balls(Simulation* s, Image* img);
void simulation_draw_field(Simulation* s, Image* img);
