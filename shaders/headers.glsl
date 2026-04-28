layout(std140, binding = BINDING_CONFIG) uniform ConfigBlock {
    uint sx;
    uint sy;
    uint sz;

    float pressure_force;
    float target_pressure;
    float particle_influence_radius;
    float radius;
    float gravity_multiplier;
    float velocity_collision_dampner;
    float velocity_drag;
    float viscosity_strength;

    uint chunk_size;

    uint nb_chunk_x;
    uint nb_chunk_y;
    uint nb_chunk_z;

    uint screen_width;
    uint screen_height;
} c;

layout(std140, binding = BINDING_SIMULATION) uniform SimulationBlock {
    float dt;

    int config_ubo;
    int simulation_ubo;
} s;
