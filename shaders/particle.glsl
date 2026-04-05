struct Particle
{
    vec2 pos;
    vec2 velo;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
    Particle particles[];
};
