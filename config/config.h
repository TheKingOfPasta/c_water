#pragma once

typedef struct
{
    int sx;
    int sy;
    int sz;


    float pressure_force;
    float target_pressure;
    float particle_influence_radius;
    float radius;
    float gravity_multiplier;
    float velocity_collision_dampner;
    float velocity_drag;
    float viscosity_strength;

    int chunk_size;

    int nb_chunk_x;
    int nb_chunk_y;
} config;
