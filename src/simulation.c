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

    Vec2 velocities[NB_PARTICULES] = { 0 };
    Vec2 positions[NB_PARTICULES];
    for (size_t i = 0; i < NB_PARTICULES; i++)
        positions[i] = s->particules[i].pos;

    for (size_t i = 0; i < NB_PARTICULES; i++)
    for (size_t j = i + 1; j < NB_PARTICULES; j++)
    {
        float dist_sqr = vec2_dist_sqrd(positions[i], positions[j]);
        if (dist_sqr < 4 * PARTICULE_RADIUS * PARTICULE_RADIUS && dist_sqr > 0.0001)
        {
            float dist = sqrtf(dist_sqr);

            Vec2 dir = vec2_sub(positions[i], positions[j]);
            float dot = vec2_dot(vec2_sub(s->particules[i].velo, s->particules[j].velo), dir);

            if (dot < 0)
            {
                Vec2 v_diff = vec2_mul_scalar(dir, dot / dist_sqr);

                v_diff = vec2_mul_scalar(v_diff, VELOCITY_COLLISION_DAMPNER);

                vec2_sub_inplace(velocities + i, v_diff);
                vec2_add_inplace(velocities + j, v_diff);
            }

            Vec2 d2 = vec2_mul_scalar(dir, (2 * PARTICULE_RADIUS - dist) / dist);

            vec2_add_inplace(positions + i, vec2_mul_scalar(d2, 0.5f));
            vec2_sub_inplace(positions + j, vec2_mul_scalar(d2, 0.5f));
        }
    }

    for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        if (vec2_add(s->particules[i].velo, velocities[i]).x > 100000 || vec2_add(s->particules[i].velo, velocities[i]).x < -100000)
        {

        vec2_print(&s->particules[i].pos);
        printf(" - ");
        vec2_print(&s->particules[i].velo);
        printf(" -> ");

        vec2_print(positions + i);
        printf(" - ");
        Vec2 res = vec2_add(s->particules[i].velo, velocities[i]);
        vec2_print(&res);
        printf("\n");
        }

        s->particules[i].pos = positions[i];
        vec2_add_inplace(&s->particules[i].velo, velocities[i]);
    }
}
