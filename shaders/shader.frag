#include "shaders/particle.glsl"
#include "shaders/chunks.h"

layout(std430, binding = BINDING_DENSITY_FIELD) buffer DensityFieldBuffer
{
    uint density_field[];
};

out vec4 FragColor;

uniform uint NB_PARTICLES;

vec3 get_dir()
{
    float x = (gl_FragCoord.x / c.screen_width) * 2.0 - 1;
    float y = (gl_FragCoord.y / c.screen_height) * 2.0 - 1;

    float aspect = c.screen_width / float(c.screen_height);

    vec3 dir = normalize(vec3(x * aspect, y, 1.0 / tan(radians(60.0) * 0.5)));

    float cosPitch = cos(s.cam_pitch);
    float sinPitch = sin(s.cam_pitch);
    float cosYaw = cos(s.cam_yaw);
    float sinYaw = sin(s.cam_yaw);

    mat3 pitchMat = mat3(
        1, 0, 0,
        0, cosPitch, sinPitch,
        0, -sinPitch, cosPitch
    );

    mat3 yawMat = mat3(
        cosYaw, 0, -sinYaw,
        0, 1, 0,
        sinYaw, 0, cosYaw
    );

    return yawMat * pitchMat * dir;
}

float next_chunk_t(vec3 pos, vec3 dir)
{
    int dx = dir.x < 0 ? -1 : 1;
    int dy = dir.y < 0 ? -1 : 1;
    int dz = dir.z < 0 ? -1 : 1;

    uint cx = clamp(uint(pos.x) / c.chunk_size, 0, c.nb_chunk_x - 1);
    uint cy = clamp(uint(pos.y) / c.chunk_size, 0, c.nb_chunk_y - 1);
    uint cz = clamp(uint(pos.z) / c.chunk_size, 0, c.nb_chunk_z - 1);

    float t_x = -1;
    float t_y = -1;
    float t_z = -1;

    if (dir.x != 0 && ((dx == 1 && cx != c.nb_chunk_x - 1) || (dx == -1 && cx != 0)))
        t_x = ((cx + dx) * c.chunk_size - pos.x) / dir.x;
    if (dir.y != 0 && ((dy == 1 && cy != c.nb_chunk_y - 1) || (dy == -1 && cy != 0)))
        t_y = ((cy + dy) * c.chunk_size - pos.y) / dir.y;
    if (dir.z != 0 && ((dz == 1 && cz != c.nb_chunk_z - 1) || (dz == -1 && cz != 0)))
        t_z = ((cz + dz) * c.chunk_size - pos.z) / dir.z;

    if (t_x == -1)
    {
        if (t_y == -1)
            return t_z;
        if (t_z == -1)
            return t_y;
        return min(t_y, t_z);
    }
    else if (t_y == -1)
    {
        if (t_z == -1)
            return t_x;
        return min(t_x, t_z);
    }
    else if (t_z == -1)
    {
        return min(t_x, t_y);
    }

    return min(min(t_x, t_y), t_z);
}

uint detect(vec3 pos, vec3 dir)
{
    int radius = 1;
    float r2 = c.radius * c.radius;

    uint cx = clamp(uint(pos.x) / c.chunk_size, 0, c.nb_chunk_x - 1);
    uint cy = clamp(uint(pos.y) / c.chunk_size, 0, c.nb_chunk_y - 1);
    uint cz = clamp(uint(pos.z) / c.chunk_size, 0, c.nb_chunk_z - 1);

    uint density = density_field[cx + cy * c.nb_chunk_x * c.nb_chunk_z + cz * c.nb_chunk_x];
    if (density == 0)
        return 0;

    for (int dx = -radius; dx <= radius; dx++)
        for (int dy = -radius; dy <= radius; dy++)
            for (int dz = -radius; dz <= radius; dz++)
            {
                if ((cx == 0 && dx < 0) || (cy == 0 && dy < 0) || (cz == 0 && dz < 0))
                    continue;

                uint nx = cx + dx;
                uint ny = cy + dy;
                uint nz = cz + dz;

                if (nx >= c.nb_chunk_x || ny >= c.nb_chunk_y || nz >= c.nb_chunk_z)
                    continue;

                uint start = start_chunks[nx + ny * c.nb_chunk_x * c.nb_chunk_z + nz * c.nb_chunk_x];
                if (start == -1 || start >= NB_PARTICLES)
                    continue;

                uint chunk_idx = pairs[start].chunk_idx;

                for (uint i = start; i < NB_PARTICLES && pairs[i].chunk_idx == chunk_idx; i++)
                {
                    vec3 diff = particles[pairs[i].particle_idx].pos - pos;
                    float sqr_dist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
                    if (sqr_dist <= c.particle_influence_radius * c.particle_influence_radius)
                        return 1;
                }
            }

    return 0;
}

void main()
{
    vec3 dir = get_dir();

    vec3 pos = s.cam_pos;

    bool is_inside = false;

    int counter = 0;

    uint particle_count = 0;
    while ((dir.x > 0 || pos.x > 0) && (dir.x <= 0 || pos.x < c.sx) &&
             (dir.y > 0 || pos.y > 0) && (dir.y <= 0 || pos.y < c.sy) &&
             (dir.z > 0 || pos.z > 0) && (dir.z <= 0 || pos.z < c.sz))
    //while ((!is_inside || (pos.x >= 0 && pos.x <= c.sx && pos.y >= 0 && pos.y <= c.sy && pos.z >= 0 && pos.z <= c.sz)) && counter < 1000)
    {
        uint found_particle = 0;

        float min_dist = 472832374.0;
        if (pos.x >= 0 && pos.x <= c.sx && pos.y >= 0 && pos.y <= c.sy && pos.z >= 0 && pos.z <= c.sz)
        {
            found_particle = detect(pos, dir);

            is_inside = true;
        }

        float t = 1;

        if (found_particle == 0)
        {
            //t = next_chunk_t(pos, dir);
        }

        pos += dir * t;

        if (is_inside)
        {
            counter += 1;
            particle_count += found_particle;
        }
    }

    vec3 max_col = vec3(0, 0, 1);
    vec3 dark = vec3(0, 0, 0.2);

    if (particle_count == 0)
    {
        FragColor = vec4(0, 0, 0, 1);
        return;
    }

    if (counter == 0)
        FragColor = vec4(1, 1, 1, 1);
    else
        FragColor = vec4(mix(dark, max_col, float(particle_count) / c.particle_density_threshold), 1);
}
