#include "config.h"

config c = {
    .sx = 1920,

#if defined (__NIXOS__)
    .sy = 1200,
#else
    .sy = 1080,
#endif

    .pressure_force = 100.01,
    .target_pressure = 0.02,
    .particule_influence_radius = 80,
    .radius = 7,
    .gravity_multiplier = 1,

    .velocity_drag = 0.999,
    .velocity_collision_dampner = 0.2,
};
