#pragma once

#include <stdint.h>
#include "utils/vec3.h"

#define NB_PARTICLES 800

typedef struct
{
    Vec3 pos;
    Vec3 velo;
} shader_particle;

typedef struct
{
    uint32_t chunk_idx;
    uint32_t particle_idx;
} chunk_particle_idx_pair;
