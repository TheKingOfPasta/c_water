#include "simulation.h"

#include <omp.h>
#include <math.h>
#include <stdlib.h>

#include "utils.h"
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

    // res.density_field = calloc(, sizeof(*res.density_field));
    return res;
}

void simulation_free([[maybe_unused]] Simulation* s)
{
    // free(s->density_field);
}

float simulation_compute_density(Simulation* s, Particule* p)
{
    float d = 0;
    const float mass = 1;

    for (int k = 0; k < NB_PARTICULES; k++)
    {
        d += particule_density(vec2_dist(s->particules[k].pos, p->pos));
    }

    return d * mass;
}

static void simulation_update_density_field(Simulation* s)
{
    for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        Particule* p = s->particules + i;
        float d = simulation_compute_density(s, p);

        s->density_field[i] = d;
    }
}

void simulation_step(Simulation* s)
{
    simulation_update_density_field(s);

#pragma omp parallel for
    for (int i = 0; i < NB_PARTICULES; i++)
    {
        particule_step(s, &s->particules[i]);
    }
}
