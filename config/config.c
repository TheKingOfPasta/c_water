#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 0.01,
    .target_pressure = 2,
    .particle_influence_radius = 20,
    .radius = 2,
    .gravity_multiplier = 0.0,

    .velocity_drag = 1.0,
    .velocity_collision_dampner = 1.0,
};
