#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif
    .sz = 1000,

    .pressure_force = 0.01,
    .target_pressure = 10,
    .particle_influence_radius = 32,
    .radius = 5,
    .gravity_multiplier = 0,
    .viscosity_strength = 10,

    .velocity_drag = 0.99,
    .velocity_collision_dampner = 0.9,
};
