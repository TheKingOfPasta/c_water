struct Particle {
    vec2 pos;
    vec2 velo;
    float r;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
    Particle particles[];
};

void main()
{
    Particle p = particles[gl_VertexID];

    vec2 v;
    v.x = p.pos.x / 1920.0 * 2.0 - 1.0;
    v.y = p.pos.y / 1080.0 * 2.0 - 1.0;

    gl_Position = vec4(v, 0.0, 1.0);
    gl_PointSize = c.radius;
}
