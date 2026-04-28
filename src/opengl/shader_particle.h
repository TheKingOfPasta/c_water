#pragma once

#include <stdint.h>
#include "utils/vec3.h"

#define NB_PARTICLES 3
#define NB_CHUNKS (c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z)

typedef struct
{
    Vec3 pos;
    float padding_0;// Since glsl vectors are converted to vec4s???
    Vec3 velo;
    float padding_1;
} shader_particle;

typedef struct
{
    uint32_t chunk_idx;
    uint32_t particle_idx;
} chunk_particle_idx_pair;
