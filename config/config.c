#include "config.h"

config c = {
    .sx = 200,
    .sy = 200,
    .sz = 200,

    .pressure_force = 1,
    .target_pressure = -1,
    .particle_influence_radius = 20,
    .radius = 5,
    .gravity_multiplier = 0,
    .viscosity_strength = 0,
    .particle_density_threshold = 1,

    .velocity_drag = 0.99,
    .velocity_collision_dampner = 0.8,

    .screen_width = 1920,

#if defined(__NIXOS__)
    .screen_height = 1200,
#else
    .screen_height = 1080,
#endif

    .nb_particles = 4,
};
