#include "shaders/particle.glsl"
#include "shaders/chunks.h"

out vec4 FragColor;

uniform int NB_PARTICLES;

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

void main()
{
    vec3 dir = get_dir();

    vec3 pos = vec3(c.sx / 2.0, c.sy / 2.0, -3);

    bool is_inside = false;

    dir = vec3(dir.x * c.nb_chunk_x, dir.y * c.nb_chunk_y, dir.z * c.nb_chunk_z) * 0.125;

    while (!is_inside || (pos.x >= 0 && pos.x <= c.sx && pos.y >= 0 && pos.y <= c.sy && pos.z >= 0 && pos.z <= c.sz))
    {
        float min_dist = 472832374.0;
        if (pos.x >= 0 && pos.x <= c.sx && pos.y >= 0 && pos.y <= c.sy && pos.z >= 0 && pos.z <= c.sz)
        {
            int cx = (int(pos.x) / c.chunk_size < 0 ? 0 : int(pos.x) / c.chunk_size >= c.nb_chunk_x ? c.nb_chunk_x - 1 : int(pos.x) / c.chunk_size);
            int cy = (int(pos.y) / c.chunk_size < 0 ? 0 : int(pos.y) / c.chunk_size >= c.nb_chunk_y ? c.nb_chunk_y - 1 : int(pos.y) / c.chunk_size);
            int cz = (int(pos.z) / c.chunk_size < 0 ? 0 : int(pos.z) / c.chunk_size >= c.nb_chunk_z ? c.nb_chunk_z - 1 : int(pos.z) / c.chunk_size);

            int start = start_chunks[cx + cy * c.nb_chunk_x * c.nb_chunk_z + cz * c.nb_chunk_x];
            if (start != -1 && start < NB_PARTICLES)
            {
                uint chunk_idx = pairs[start].chunk_idx;
                for (int i = start; i < NB_PARTICLES && pairs[i].chunk_idx == chunk_idx; i++)
                {
                    vec3 diff = particles[i].pos - pos;
                    float sqr_dist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
                    if (sqr_dist <= c.radius * c.radius)
                    {
                        FragColor = vec4(1, 1, 1, 1);
                        return;
                    }
                    else if (sqr_dist < min_dist)
                        min_dist = sqr_dist;
                }

                pos += dir * sqrt(min_dist);
            }
            else
                pos += dir;

            is_inside = true;
        }
        else
            pos += dir;
    }

    FragColor = vec4(0, 0, 0, 1);
}
