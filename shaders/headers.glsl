layout(std140, binding = BINDING_CONFIG) uniform ConfigBlock {
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
} c;

layout(std140, binding = BINDING_SIMULATION) uniform SimulationBlock {
    float dt;

    int config_ubo;
    int simulation_ubo;
} s;
