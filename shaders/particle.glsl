struct Particle
{
    vec2 pos;
    vec2 velo;
};

layout(std430, binding = BINDING_PARTICLES) buffer ParticleBuffer
{
    Particle particles[];
};
