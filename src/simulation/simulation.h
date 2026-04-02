#pragma once

#include "image/image.h"
#include "utils/vec2.h"

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particle;

#define NB_PARTICLES 10000

#define CHUNK_SIZE_SCALE_COMPARED_TO_PARTICLE_RADIUS 6

typedef struct
{
    uint16_t chunk_idx;
    uint16_t particle_idx;
} chunk_particle_idx_pair;

#define CHUNK_EMPTY_IDX 0xFFFF

typedef struct
{
    Particle particles[NB_PARTICLES];

    float particle_densities[NB_PARTICLES];

    int16_t chunk_size;
    chunk_particle_idx_pair pairs[NB_PARTICLES];

    int nb_chunk_x;
    int nb_chunk_y;
    uint16_t* start_chunk;
    uint16_t* end_chunk;

    float dt;
} Simulation;

/* --- PARTICLES --- */
Particle particle_gen_random();

void particle_step(Simulation* s, size_t index);

void particle_print(Particle* p);

float particle_density(float d);

/* --- SIMULATION --- */

Simulation simulation_gen();
void simulation_free(Simulation* s);

void simulation_step(Simulation* s);
void simulation_update_chunks(Simulation* s);

float simulation_compute_density(Simulation* s, Particle* p);

void simulation_draw_density(Simulation* s, Image* img);
void simulation_draw_balls(Simulation* s, Image* img);
void simulation_draw_field(Simulation* s, Image* img);
void simulation_draw_chunks(Simulation* s, Image* img, float x, float y);
void simulation_print_chunks(Simulation* s);
