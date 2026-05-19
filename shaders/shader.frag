#include "shaders/chunks.h"
#include "shaders/particle.glsl"

layout(std430, binding = BINDING_EXISTENCE_FIELD) buffer ExistenceFieldBuffer
{
    uint existence_field[];
};

out vec4 FragColor;

#define MAX_STEPS 100
#define STEP_LEN 25.0
#define SURFACE_THRESHOLD 0.8 // beetween 0 and 1 // K-value
#define SURFACE_SEARCH_ITERATION 1

#define INNER_STEPS (MAX_STEPS/2)
#define INNER_STEP_LEN 30.0 // higher steps size for sub marching

#define FLUID_TINT vec3(1.0, 1.0, 0.0) // absorption colour of the water
#define ABSORPTION 0.01 // higher = less transparent
#define INDEX_OF_REFLECTION 1.333
#define BASE_REFLECTANCE 0.02
#define REFLECTION_GAIN 1.0 // scale the reflected sky contribution
#define SPECULAR_POWER 1000.0
#define SPECULAR_GAIN 1.4

#define LIGHT_DIR normalize(vec3(0.45, 0.85, 0.30))
#define LIGHT_COLOR vec3(1.0, 0.97, 0.92)
#define AMBIENT_LIGHT vec3(0.18, 0.22, 0.28)

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

    mat3 pitchMat = mat3(1, 0, 0, 0, cosPitch, sinPitch, 0, -sinPitch, cosPitch);

    mat3 yawMat = mat3(cosYaw, 0, -sinYaw, 0, 1, 0, sinYaw, 0, cosYaw);

    return yawMat * pitchMat * dir;
}

vec3 background_sky(vec3 dir)
{
    vec3 skyBottom = vec3(54.0, 98.0, 227.0) / 255.0;
    vec3 skyTop = vec3(168.0, 183.0, 227.0) / 255.0;

    float t = smoothstep(0.0, 1.0, (dir.y + 1.0) * 0.5);
    return mix(skyBottom, skyTop, t);
}

vec3 background(vec3 pos, vec3 dir)
{
    if (dir.y < -0.0001)
    {
        float ground_height = -10;
        float t = (ground_height - pos.y) / dir.y;
        if (t > 0.0)
        {
            vec3 hit = pos + dir * t;

            float grid_size = 42;
            vec3 grid_col_A = vec3(0.1, 0.1, 0.1);
            vec3 grid_col_B = vec3(0.8, 0.8, 0.8);

            float cx = floor(hit.x / grid_size);
            float cz = floor(hit.z / grid_size);
            vec3 base = mix(grid_col_A, grid_col_B, mod(cx + cz, 2.0));

            float lit = max(dot(vec3(0, 1, 0), LIGHT_DIR), 0.0);
            base *= AMBIENT_LIGHT + LIGHT_COLOR * lit * 0.8;
            float haze = clamp(t / 1000.0, 0.0, 1.0);
            return mix(base, background_sky(dir), haze);
        }
    }

    vec3 col = background_sky(dir);
    float sun = max(dot(dir, LIGHT_DIR), 0.0);
    col += LIGHT_COLOR * pow(sun, 350.0) * 1.2;
    col += LIGHT_COLOR * pow(sun, 8.0) * 0.18;

    return col;
}

bool inside_box(vec3 p)
{
    return p.x >= 0.0 && p.x <= c.sx
        && p.y >= 0.0 && p.y <= c.sy
        && p.z >= 0.0 && p.z <= c.sz;
}

float field(vec3 pos)
{
    uint cx = clamp(uint(pos.x) / c.chunk_size, 0u, c.nb_chunk_x - 1u);
    uint cy = clamp(uint(pos.y) / c.chunk_size, 0u, c.nb_chunk_y - 1u);
    uint cz = clamp(uint(pos.z) / c.chunk_size, 0u, c.nb_chunk_z - 1u);

    if (existence_field[cx + cy * c.nb_chunk_x * c.nb_chunk_z + cz * c.nb_chunk_x] == 0u)
        return 0.0;

    float r  = c.particle_influence_radius;
    float r2 = r * r;
    float sum = 0.0;

    int lo_x = int((pos.x - r) / float(c.chunk_size));
    int hi_x = int((pos.x + r) / float(c.chunk_size));
    int lo_y = int((pos.y - r) / float(c.chunk_size));
    int hi_y = int((pos.y + r) / float(c.chunk_size));
    int lo_z = int((pos.z - r) / float(c.chunk_size));
    int hi_z = int((pos.z + r) / float(c.chunk_size));

    lo_x = max(lo_x, 0);  hi_x = min(hi_x, int(c.nb_chunk_x) - 1);
    lo_y = max(lo_y, 0);  hi_y = min(hi_y, int(c.nb_chunk_y) - 1);
    lo_z = max(lo_z, 0);  hi_z = min(hi_z, int(c.nb_chunk_z) - 1);

    for (int nx = lo_x; nx <= hi_x; nx++)
        for (int ny = lo_y; ny <= hi_y; ny++)
            for (int nz = lo_z; nz <= hi_z; nz++)
            {
                uint start = start_chunks[uint(nx) + uint(ny) * c.nb_chunk_x * c.nb_chunk_z
                                          + uint(nz) * c.nb_chunk_x];
                if (start == uint(-1) || start >= NB_PARTICLES)
                    continue;

                uint chunk_idx = pairs[start].chunk_idx;

                for (uint i = start; i < NB_PARTICLES && pairs[i].chunk_idx == chunk_idx; i++)
                {
                    vec3 d = particles[pairs[i].particle_idx].pos - pos;
                    float sqr = dot(d, d);
                    if (sqr < r2)
                    {
                        float x = 1.0 - sqr / r2;
                        sum += x * x * x;
                    }
                }
            }

    return sum;
}

vec3 field_normal(vec3 p)
{
    float e = 0.9;
    vec3 g = vec3(
        field(p + vec3(e, 0, 0)) - field(p - vec3(e, 0, 0)),
        field(p + vec3(0, e, 0)) - field(p - vec3(0, e, 0)),
        field(p + vec3(0, 0, e)) - field(p - vec3(0, 0, e)));

    if (dot(g, g) < 1e-12)
        return vec3(0, 1, 0);
    return normalize(-g);
}

vec3 trace_inside(vec3 ro, vec3 rd)
{
    vec3 p = ro;
    float traveled = 0.0;
    bool was_inside = true;

    for (int i = 0; i < INNER_STEPS; i++)
    {
        p += rd * INNER_STEP_LEN;

        if (!inside_box(p))
            break;

        bool in_fluid = field(p) > SURFACE_THRESHOLD;
        if (in_fluid)
            traveled += INNER_STEP_LEN;
        was_inside = in_fluid;
    }

    vec3 back = background(p, rd);

    // absorb more the more traveled
    vec3 absorb = exp(-FLUID_TINT * ABSORPTION * traveled * 6.0);
    return back * absorb;
}

void main()
{
    vec3 dir = get_dir();

    vec3 pos_or = s.cam_pos;
    vec3 pos = s.cam_pos;

    vec3 surf_pos    = vec3(0.0);

    bool hit_smthng = false;

    float prev_f      = field(pos);
    vec3  prev_pos    = pos;

    for (int i = 0; i < MAX_STEPS; i++)
    {
        if ((dir.x <= 0.0 && pos.x < 0.0) || (dir.x >= 0.0 && pos.x > c.sx)
         || (dir.y <= 0.0 && pos.y < 0.0) || (dir.y >= 0.0 && pos.y > c.sy)
         || (dir.z <= 0.0 && pos.z < 0.0) || (dir.z >= 0.0 && pos.z > c.sz))
            break;

        float f = inside_box(pos) ? field(pos) : 0.0;

        if (f > SURFACE_THRESHOLD && prev_f <= SURFACE_THRESHOLD)
        {
            hit_smthng = true;

			vec3 a = prev_pos;
            vec3 b = pos;
            for (int k = 0; k < SURFACE_SEARCH_ITERATION; k++)
            {
                vec3 m = (a + b) * 0.5;
                if (field(m) > SURFACE_THRESHOLD)
                    b = m;
                else
                    a = m;
            }
            surf_pos = b;
            break;
        }

        prev_f   = f;
        prev_pos = pos;
        pos     += dir * STEP_LEN;
    }

    if (!hit_smthng)
    {
        FragColor = vec4(background(pos_or, dir), 1.0);
        return;
    }

    vec3 N = field_normal(surf_pos);
    vec3 V = -dir;

    if (dot(N, V) < 0.0)
        N = -N;

    // Fresnel : reflect vs refract
    float cosTheta = clamp(dot(N, V), 0.0, 1.0);
    float fres = BASE_REFLECTANCE + (1.0 - BASE_REFLECTANCE) * pow(1.0 - cosTheta, 5.0);

    // reflection: bounce the view ray into the sky
    vec3 R = reflect(dir, N);
    vec3 reflection = background(surf_pos + N * 0.5, R) * REFLECTION_GAIN;

    // transparency: refract in the fluid
    vec3 T = refract(dir, N, 1.0 / INDEX_OF_REFLECTION);
    vec3 refraction;
    if (dot(T, T) < 1e-8)
        refraction = reflection;
    else
        refraction = trace_inside(surf_pos - N * 0.5, normalize(T));

    // specular
    vec3 H = normalize(LIGHT_DIR + V);
    float spec = pow(max(dot(N, H), 0.0), SPECULAR_POWER);
    vec3 specular = LIGHT_COLOR * spec * SPECULAR_GAIN;

    // diffuse
    float diff = max(dot(N, LIGHT_DIR), 0.0);
    vec3 body = (AMBIENT_LIGHT + LIGHT_COLOR * diff * 0.25) * FLUID_TINT;

    vec3 color = mix(refraction, reflection, fres); // Fresnel reflect & refract
    color += body * (1.0 - fres) * 0.4;
    color += specular;

    // tonemap
    color = color / (color + vec3(0.6));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
