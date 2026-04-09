#include "shaders/particle.glsl"

out vec4 FragColor;

uniform int NB_PARTICLES;

vec3 get_dir()
{
    float x = (gl_FragCoord.x / c.sx) * 2.0 - 1;
    float y = (gl_FragCoord.y / c.sy) * 2.0 - 1;

    vec3 right = vec3(1, 0, 0);
    vec3 up = vec3(0, 1, 0);
    vec3 di = vec3(0, 0, 1);

    float aspect = c.sx / float(c.sy);

    vec3 dir = normalize(right * x * aspect + up * y + di / tan(radians(80) * 0.5));
    return dir;
}

bool hit_particle(vec3 pos, vec3 dir)
{
    vec3 cam_pos = vec3(c.sx / 2.0, c.sy / 2.0, -300);
    vec3 oc = cam_pos - pos;

    float A = dot(dir, dir);
    float B = 2.0 * dot(oc, dir);
    float C = dot(oc, oc) - c.radius * c.radius;

    float D = B * B - 4 * A * C;

    if (D < 0)
        return false;

    float sqrtD = sqrt(D);

    float t1 = (-B - sqrtD) / (2 * A);
    float t2 = (-B + sqrtD) / (2 * A);

    return t1 >= 0 || t2 >= 0;
}

void main()
{
    vec3 dir = get_dir();

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        if (hit_particle(particles[i].pos, dir))
        {
            FragColor = vec4(1, 1, 1, 1);
            return;
        }
    }

    FragColor = vec4(0, 0, 0, 1);
}
