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

float simulation_compute_density(Simulation* s, Vec2 pos)
{
    float d = 0;
    const float mass = 100;

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
    float i_dx = 0.5f;
    float i_dy = 0.5f;

    float xm = x - 1;
    float xp = x + 1;
    float ym = y - 1;
    float yp = y + 1;

    if (xm < 0)
    {
        xm = x;
        i_dx = 1.0f;
    }
    else if (xp > s->sx - 1)
    {
        xp = s->sx - 1;
        i_dx = 1.0f;
    }

    if (ym < 0)
    {
        ym = y;
        i_dy = 1.0f;
    }
    else if (yp > s->sy - 1)
    {
        yp = s->sy - 1;
        i_dy = 1.0f;
    }

    float d_xm = simulation_compute_density(s, (Vec2){ .x = xm, .y = y });
    float d_xp = simulation_compute_density(s, (Vec2){ .x = xp, .y = y });
    float d_ym = simulation_compute_density(s, (Vec2){ .x = x, .y = ym });
    float d_yp = simulation_compute_density(s, (Vec2){ .x = x, .y = yp });

    return (Vec2){ (d_xp - d_xm) * i_dx, (d_yp - d_ym) * i_dy };
}

void simulation_step(Simulation* s)
{
    simulation_update_density_field(s);
    for (int i = 0; i < NB_PARTICULES; i++)
    {
        particule_step(s, &s->particules[i]);
    }
}
