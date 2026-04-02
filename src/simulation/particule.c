#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "config_reloader.h"
#include "simulation.h"
#include "utils/utils.h"
#include "utils/vec2.h"

void particule_print(Particule* p)
{
    printf("{ pos:");
    vec2_print(&p->pos);
    printf(", velo:");
    vec2_print(&p->velo);
    printf(" }\n");
}

Particule particule_gen_random()
{
    Particule p = { .pos = vec2_random(), .velo = vec2_zero() };
    p.pos.x *= c->sx;
    p.pos.y *= c->sy;
    return p;
}

static inline void particule_interact_bounds(Particule* p)
{
    if (p->pos.x < 0)
    {
        p->velo.x *= -1;
        p->velo = vec2_mul_scalar(p->velo, c->velocity_collision_dampner);
        p->pos.x = -p->pos.x;
        if (p->pos.x > c->sx)
            p->pos.x = 0;
    }
    else if (p->pos.x >= c->sx)
    {
        p->velo.x *= -1;
        p->velo = vec2_mul_scalar(p->velo, c->velocity_collision_dampner);
        p->pos.x = c->sx - (p->pos.x - c->sx);
        if (p->pos.x < 0)
            p->pos.x = c->sx;
    }

    if (p->pos.y < 0)
    {
        p->velo.y *= -1;
        p->velo = vec2_mul_scalar(p->velo, c->velocity_collision_dampner);
        p->pos.y = -p->pos.y;
        if (p->pos.y > c->sy)
            p->pos.y = 0;
    }
    else if (p->pos.y >= c->sy)
    {
        p->velo.y *= -1;
        p->velo = vec2_mul_scalar(p->velo, c->velocity_collision_dampner);
        p->pos.y = c->sy - (p->pos.y - c->sy);
        if (p->pos.y < 0)
            p->pos.y = c->sy;
    }
}

static inline void particule_apply_gravity(Particule* p)
{
    vec2_add_inplace(&p->velo, (Vec2){ 0, 0.0981f * c->gravity_multiplier });
}

void particule_step(Simulation* s, Particule* p)
{
    float pressure = simulation_compute_density(s, p) - c->target_pressure;

    Vec2 grad = particule_compute_gradient(s, p);

    grad = vec2_neg(grad);

    particule_apply_gravity(p);

    vec2_add_inplace(&p->velo,
                     vec2_mul_scalar(grad, pressure * c->pressure_force));

    vec2_add_inplace(&p->pos, p->velo);
    p->velo = vec2_mul_scalar(p->velo, c->velocity_drag);

    particule_interact_bounds(p);
}

static float
particule_compute_density_gradient(float dist, float particule_influence_radius)
{
    float slope = 6.0 / (M_PI * pow(particule_influence_radius, 4));

    return 2 * slope * (dist - particule_influence_radius);
}

Vec2 particule_compute_gradient(Simulation* s, Particule* p)
{
    Vec2 res = vec2_zero();

    for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        Particule* p2 = s->particules + i;
        if (p2 == p
            || vec2_dist_sqrd(p2->pos, p->pos)
                > c->particule_influence_radius * c->particule_influence_radius)
            continue;

        float density = s->particle_densities[i];
        if (density < 0.001)
            continue;

        Vec2 dir = vec2_sub(s->particules[i].pos, p->pos);
        float dist = vec2_dist(p2->pos, p->pos);
        if (dist < 0.001)
            continue;
        dir = vec2_mul_scalar(dir, 1.0f / dist);

        float slope = particule_compute_density_gradient(
            dist, c->particule_influence_radius);

        vec2_add_inplace(&res, vec2_mul_scalar(dir, slope / density));
    }

    return res;
}

float particule_density(float d)
{
    const float vol = 6.0 / (M_PI * c->particule_influence_radius * c->particule_influence_radius * c->particule_influence_radius * c->particule_influence_radius);

    float v = c->particule_influence_radius - d;

    if (v < 0)
        return 0;

    return v * v * vol;
}
