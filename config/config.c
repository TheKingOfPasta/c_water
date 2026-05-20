#include "config.h"

config c = {
    .sx = 2000,
    .sy = 1000,
    .sz = 2000,

    .pressure_force = 5,
    .target_pressure = -1,
    .particle_influence_radius = 40,
    .radius = 5,
    .gravity_multiplier = 100,
    .viscosity_strength = 0,
    .particle_density_threshold = 1,

    .velocity_drag = 0.98,
    .velocity_collision_dampner = 0.6,

    .static_config = {
        .screen_width = 1920,
#if defined(__NIXOS__)
        .screen_height = 1200,
#else
        .screen_height = 1080,
#endif
        .render_scale = 0.5,

    	.nb_particles = 27000,
    },

    .cam_move_speed = 150.0,
    .cam_mouse_sensitivity = 0.0025,
    .sim_speed = 6.0,

};
