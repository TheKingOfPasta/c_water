#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 0.001,
    .target_pressure = -60,
    .particle_influence_radius = 10,
    .radius = 1,
    .gravity_multiplier = 0.0,

    .velocity_drag = 1,
    .velocity_collision_dampner = 0.9,
};
