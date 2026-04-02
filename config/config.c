#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 5,
    .target_pressure = 0,
    .particle_influence_radius = 20,
    .radius = 4,
    .gravity_multiplier = 0.0,

    .velocity_drag = 1.0,
    .velocity_collision_dampner = 1.0,
};
