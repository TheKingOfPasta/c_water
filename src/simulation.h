#pragma once

#include "image.h"
#include "vec2.h"

#define PRESSURE_FORCE 10000
#define TARGET_PRESSURE 0.015
#define PARTICULE_INFLUENCE_RADIUS 30 // for density
#define PARTICULE_RADIUS 10 // for collision & drawing
#define GRAVITY_MULTIPLIER 0.0
#define VELOCITY_COLLISION_DAMPNER 0.0 // 0.0 no loss - 1.0 100% loss
#define DRAG 0.9

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particule;

#define NB_PARTICULES 2000

#define CHUNK_SIZE (PARTICULE_RADIUS * 6)

typedef struct
{
    uint16_t chunk_idx;
    uint16_t particle_idx;
} chunk_particle_idx_pair;

#define CHUNK_EMPTY_IDX 0xFFFF

typedef struct
{
    int sx;
    int sy;

    Particule particules[NB_PARTICULES];

    float particle_densities[NB_PARTICULES];

    chunk_particle_idx_pair pairs[NB_PARTICULES];

    int nb_chunk_x;
    int nb_chunk_y;
    uint16_t* start_chunk;
    uint16_t* end_chunk;
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
void simulation_update_chunks(Simulation* s);

float simulation_compute_density(Simulation* s, Particule* p);

void simulation_draw_balls(Simulation* s, Image* img);
void simulation_draw_field(Simulation* s, Image* img);
void simulation_draw_chunks(Simulation* s, Image* img, float x, float y);
void simulation_print_chunks(Simulation* s);
