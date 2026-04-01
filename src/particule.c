#include <math.h>
#include <stdio.h>

#include "simulation.h"
#include "utils.h"
#include "vec2.h"

void particule_print(Particule* p)
{
    printf("{ pos:");
    vec2_print(&p->pos);
    printf(", velo:");
    vec2_print(&p->velo);
    printf(" }\n");
}

Particule particule_gen_random(int sx, int sy)
{
    Particule p = { .pos = vec2_random(), .velo = vec2_zero() };
    p.pos.x *= sx;
    p.pos.y *= sy;
    return p;
}

static inline void particule_interact_bounds(Particule* p, int sx, int sy)
{
    if (p->pos.x < 0)
    {
        p->velo.x *= -1;
        p->velo = vec2_mul_scalar(p->velo, 1 - VELOCITY_COLLISION_DAMPNER);
        p->pos.x = -p->pos.x;
        if (p->pos.x > sx)
            p->pos.x = 0;
    }
    else if (p->pos.x >= sx)
    {
        p->velo.x *= -1;
        p->velo = vec2_mul_scalar(p->velo, 1 - VELOCITY_COLLISION_DAMPNER);
        p->pos.x = sx - (p->pos.x - sx);
        if (p->pos.x < 0)
            p->pos.x = sx;
    }

    if (p->pos.y < 0)
    {
        p->velo.y *= -1;
        p->velo = vec2_mul_scalar(p->velo, 1 - VELOCITY_COLLISION_DAMPNER);
        p->pos.y = -p->pos.y;
        if (p->pos.y > sy)
            p->pos.y = 0;
    }
    else if (p->pos.y >= sy)
    {
        p->velo.y *= -1;
        p->velo = vec2_mul_scalar(p->velo, 1 - VELOCITY_COLLISION_DAMPNER);
        p->pos.y = sy - (p->pos.y - sy);
        if (p->pos.y < 0)
            p->pos.y = sy;
    }
}

static inline void particule_apply_gravity(Particule* p)
{
    vec2_add_inplace(&p->velo, (Vec2){ 0, 0.0981f * GRAVITY_MULTIPLIER });
}

void particule_step(Simulation* s, Particule* p)
{
    float pressure = simulation_compute_density(s, p) - TARGET_PRESSURE;

    Vec2 grad = particule_compute_gradient(s, p);

    /*for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        Particule* p2 = s->particules + i;

        float dist_sqr = vec2_dist_sqrd(p->pos, p2.pos);
        if (dist_sqr < PARTICULE_RADIUS * PARTICULE_RADIUS)
        {
            float dot = vec2_dot(p->velo, p2->velo);


        }
    }*/

    grad = vec2_neg(grad);

    vec2_add_inplace(&p->velo,
                     vec2_mul_scalar(grad, pressure * PRESSURE_FORCE));

    vec2_add_inplace(&p->pos, p->velo);
    p->velo = vec2_mul_scalar(p->velo, DRAG);

    particule_interact_bounds(p, s->sx, s->sy);
    particule_apply_gravity(p);
}

static float particule_compute_density_gradient(float dist)
{
    float slope = 6.0 / (M_PI * pow(PARTICULE_INFLUENCE_RADIUS, 4));

    return 2 * slope * (dist - PARTICULE_INFLUENCE_RADIUS);
}

Vec2 particule_compute_gradient(Simulation* s, Particule* p)
{
    Vec2 res = vec2_zero();

    for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        Particule* p2 = s->particules + i;
        if (p2 == p
            || vec2_dist_sqrd(p2->pos, p->pos)
                > PARTICULE_INFLUENCE_RADIUS * PARTICULE_INFLUENCE_RADIUS)
            continue;

        float density = s->particle_densities[i];

        Vec2 dir = vec2_sub(s->particules[i].pos, p->pos);
        float dist = vec2_dist(p2->pos, p->pos);
        dir = vec2_mul_scalar(dir, 1.0f / dist);

        float slope = particule_compute_density_gradient(dist);

        vec2_add_inplace(&res, vec2_mul_scalar(dir, slope / density));
    }

    return res;
}

float particule_density(float d)
{
    const float vol = 6.0 / (M_PI * pow(PARTICULE_INFLUENCE_RADIUS, 4));

    if (d > PARTICULE_INFLUENCE_RADIUS)
        return 0;

    float v = PARTICULE_INFLUENCE_RADIUS - d;
    return v * v * vol;
}
