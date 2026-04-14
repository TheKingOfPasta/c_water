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
    .target_pressure = -2,
    .particle_influence_radius = 50,
    .radius = 15,
    .gravity_multiplier = 000,
    .viscosity_strength = 0, // 0.0000000001,

    .velocity_drag = 1.0,
    .velocity_collision_dampner = 1.0,
};
