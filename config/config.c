#include "config.h"

config c = {
    .sx = 1920,

#if defined (__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 30,
    .target_pressure = 1,
    .particule_influence_radius = 50,
    .radius = 10,
    .gravity_multiplier = 0,

    .velocity_drag = 0.99,
    .velocity_collision_dampner = 0.7,
};
