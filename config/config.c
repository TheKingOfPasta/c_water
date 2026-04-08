#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 0.3,
    .target_pressure = -2,
    .particle_influence_radius = 10,
    .radius = 1,
    .gravity_multiplier = 0.2,
    .viscosity_strength = 0, // 0.0000000001,

    .velocity_drag = 0.995,
    .velocity_collision_dampner = 0.9,
};
