#include "simulation.h"

#include <stdlib.h>

#include "vec2.h"

Simulation simulation_gen(int sx, int sy)
{
    Simulation res = {
        .sx = sx,
        .sy = sy,
    };

    for (int i = 0; i < NB_PARTICULES; i++)
    {
        res.particules[i] = particule_gen_random(sx, sy);
    }

    res.density_field = calloc(sx * sy, sizeof(*res.density_field));
    return res;
}

void simulation_free(Simulation* s)
{
    free(s->density_field);
}

static float simulation_compute_density(Simulation* s, Vec2 pos)
{
    float d = 0;
    const float mass = 1000;

    for (int k = 0; k < NB_PARTICULES; k++)
    {
        d += particule_density(&s->particules[k], pos);
    }

    return d * mass;
}

static void simulation_update_density_field(Simulation* s)
{
    for (int i = 0; i < s->sx; i++)
        for (int j = 0; j < s->sy; j++)
        {
            float d = simulation_compute_density(s, (Vec2){ i, j });
            s->density_field[i + j * s->sx] = d;
        };
}

Vec2 simulation_compute_gradient(Simulation* s, float x, float y)
{
    float xm = (x > 0) ? x - 1 : x;
    float xp = (x < s->sx - 1) ? x + 1 : x;
    float ym = (y > 0) ? y - 1 : y;
    float yp = (y < s->sy - 1) ? y + 1 : y;

    float d_xm = s->density_field[(int)(y * s->sx + xm)];
    float d_xp = s->density_field[(int)(y * s->sx + xp)];
    float d_ym = s->density_field[(int)(ym * s->sx + x)];
    float d_yp = s->density_field[(int)(yp * s->sx + x)];

    return vec2_mul_scalar((Vec2){ d_xp - d_xm, d_yp - d_ym }, 0.5f);
}

void simulation_step(Simulation* s)
{
    simulation_update_density_field(s);
    for (int i = 0; i < NB_PARTICULES; i++)
    {
        particule_step(s, &s->particules[i]);
    }
}
