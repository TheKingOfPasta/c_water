#include "config.h"

config c = {
    .sx = 1920,

#if defined(__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 0.5,
    .target_pressure = -20,
    .particle_influence_radius = 10,
    .radius = 2,
    .gravity_multiplier = 4.0,

    .velocity_drag = 1.0,
    .velocity_collision_dampner = 0.25,
};
