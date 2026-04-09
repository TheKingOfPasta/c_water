struct Particle
{
    vec3 pos;
    vec3 velo;
};

layout(std430, binding = BINDING_PARTICLES) buffer ParticleBuffer
{
    Particle particles[];
};
