#pragma once

#include <stdint.h>
#include "utils/vec3.h"
#include "utils/vec2.h"

#define NB_PARTICLES 8000

typedef struct
{
    Vec2 pos;
    Vec2 velo;
} shader_particle;

typedef struct
{
    uint32_t chunk_idx;
    uint32_t particle_idx;
} chunk_particle_idx_pair;
