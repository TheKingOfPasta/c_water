#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 300,
    .target_pressure = -50,
    .particle_influence_radius = 10,
    .radius = 5,
    .gravity_multiplier = 3000,
    .viscosity_strength = 10,

    .velocity_drag = 1,
    .velocity_collision_dampner = 0.9,
};
