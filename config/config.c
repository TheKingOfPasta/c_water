#include "config.h"

config c = {
    .sx = 1000,
    .sy = 500,
    .sz = 1000,

    .pressure_force = 1,
    .target_pressure = -1,
    .particle_influence_radius = 50,
    .radius = 5,
    .gravity_multiplier = 100,
    .viscosity_strength = 0,
    .particle_density_threshold = 1,

    .velocity_drag = 0.8,
    .velocity_collision_dampner = 0.8,

    .static_config = {
        .screen_width = 1920,
#if defined(__NIXOS__)
        .screen_height = 1200,
#else
        .screen_height = 1080,
#endif
        .render_scale = 0.1,

    	.nb_particles = 27000,
    },

    .cam_move_speed = 150.0,
    .cam_mouse_sensitivity = 0.0025,
    .sim_speed = 1.0,

};
