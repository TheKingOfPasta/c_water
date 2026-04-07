#pragma once

#include "image/image.h"
#include "utils/vec2.h"

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} Particle;

#define NB_PARTICLES 80000

#define CHUNK_SIZE_SCALE_COMPARED_TO_PARTICLE_RADIUS 6

typedef struct
{
    uint32_t chunk_idx;
    uint32_t particle_idx;
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

    uint32_t* start_chunk;
    uint32_t* end_chunk;
} Simulation;

/* --- PARTICLES --- */
Particle particle_gen_random();

// void particle_step(Simulation* s, Vec2* predicted_positions, size_t index);
Vec2 particle_compute_pressure(Simulation* s, Vec2 predicted_positions[NB_PARTICLES], size_t p1_index);
void particle_interact_bounds(Particle* p);

void particle_print(Particle* p);

float particle_density(float d);

/* --- SIMULATION --- */

Simulation simulation_gen();
void simulation_free(Simulation* s);

void simulation_step(Simulation* s);
//void simulation_update_chunks(Simulation* s);

float simulation_compute_density(Simulation* s, Vec2 predicted_positions[NB_PARTICLES], size_t index);

void simulation_draw_density(Simulation* s, Image* img);
void simulation_draw_balls(Simulation* s, Image* img);
void simulation_draw_field(Simulation* s, Image* img);
void simulation_draw_chunks(Simulation* s, Image* img, float x, float y);
void simulation_print_chunks(Simulation* s);
