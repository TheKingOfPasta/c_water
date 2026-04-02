#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "config_reloader.h"
#include "image/image_drawing.h"
#include "simulation.h"
#include "utils/utils.h"
#include "utils/vec2.h"

void particle_print(Particle* p)
{
    printf("{ pos:");
    vec2_print(&p->pos);
    printf(", velo:");
    vec2_print(&p->velo);
    printf(" }\n");
}

Particle particle_gen_random()
{
    Particle p = { .pos = vec2_random(), .velo = vec2_zero() };
    p.pos.x *= c->sx;
    p.pos.y *= c->sy;
    return p;
}

static inline void particle_interact_bounds(Particle* p)
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

static inline void particle_apply_gravity(Particle* p)
{
    vec2_add_inplace(&p->velo, (Vec2){ 0, 0.0981f * c->gravity_multiplier });
}

void particle_step(Simulation* s, Particle* p)
{
    float pressure = simulation_compute_density(s, p) - c->target_pressure;

    Vec2 grad = particle_compute_gradient(s, p);

    grad = vec2_neg(grad);

    particle_apply_gravity(p);

    vec2_add_inplace(&p->velo,
                     vec2_mul_scalar(grad, pressure * c->pressure_force));

    vec2_add_inplace(&p->pos, p->velo);
    p->velo = vec2_mul_scalar(p->velo, c->velocity_drag);

    particle_interact_bounds(p);
}

static float particle_compute_density_gradient(float dist)
{
    float slope = 2.0 * 6.0 / (M_PI * c->particle_influence_radius * c->particle_influence_radius * c->particle_influence_radius * c->particle_influence_radius);

    return slope * (dist - c->particle_influence_radius);
}

Vec2 particle_compute_gradient(Simulation* s, Particle* p)
{
    Vec2 res = vec2_zero();

    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p2 = s->particles + i;
        Vec2 dir = vec2_sub(p2->pos, p->pos);
        float dist_sqrd = vec2_norm_sqrd(dir);

        if (p2 == p
            || dist_sqrd > c->particle_influence_radius * c->particle_influence_radius)
            continue;

        float density = s->particle_densities[i];
        if (density < 0.001)
            continue;

        float dist = sqrt(dist_sqrd);
        if (dist < 0.001)
            continue;
        dir = vec2_mul_scalar(dir, 1.0f / dist);

        float slope = particle_compute_density_gradient(dist);

        vec2_add_inplace(&res, vec2_mul_scalar(dir, slope / density));
    }

    return res;
}

float particle_density(float d)
{
    float v = c->particle_influence_radius - d;

    if (v < 0)
        return 0;

    float vol = 6.0 / (M_PI * c->particle_influence_radius * c->particle_influence_radius * c->particle_influence_radius * c->particle_influence_radius);

    return v * v * vol;
}
