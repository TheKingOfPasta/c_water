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
        d += particule_density(vec2_dist(s->particules[k].pos, p->pos), s->particule_influence_radius);
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

static void simulation_get_file_variables(Simulation* s)
{
#if defined(__NIXOS__)
    FILE* f_pressure_force = fopen("/home/zazou/pressure_force.txt", "r");
    FILE* f_target_pressure = fopen("/home/zazou/target_pressure.txt", "r");
    FILE* f_influence = fopen("/home/zazou/influence.txt", "r");
    FILE* f_radius = fopen("/home/zazou/radius.txt", "r");
    FILE* f_gravity = fopen("/home/zazou/gravity.txt", "r");
#else
    FILE* f_pressure_force = fopen("/home/aurel/pressure_force.txt", "r");
    FILE* f_target_pressure = fopen("/home/aurel/target_pressure.txt", "r");
    FILE* f_influence = fopen("/home/aurel/influence.txt", "r");
    FILE* f_radius = fopen("/home/aurel/radius.txt", "r");
    FILE* f_gravity = fopen("/home/aurel/gravity.txt", "r");
#endif

    char* file_pressure_force = read_all_file(f_pressure_force);
    char* file_target_pressure = read_all_file(f_target_pressure);
    char* file_influence = read_all_file(f_influence);
    char* file_radius = read_all_file(f_radius);
    char* file_gravity = read_all_file(f_gravity);

    s->pressure_force = atof(file_pressure_force);
    s->target_pressure = atof(file_target_pressure);
    s->particule_influence_radius = atof(file_influence);
    s->radius = atof(file_radius);
    s->gravity_multiplier = atof(file_gravity);

    free(file_pressure_force);
    free(file_target_pressure);
    free(file_influence);
    free(file_radius);
    free(file_gravity);
    fclose(f_pressure_force);
    fclose(f_target_pressure);
    fclose(f_influence);
    fclose(f_radius);
    fclose(f_gravity);
}
void simulation_step(Simulation* s)
{
    simulation_get_file_variables(s);

    simulation_update_density_field(s);

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        particule_step(s, &s->particules[i]);
    }

    Vec2 velocities[NB_PARTICULES] = { 0 };

    Vec2 positions[NB_PARTICULES];
#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICULES; i++)
        positions[i] = s->particules[i].pos;

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICULES; i++)
    for (size_t j = i + 1; j < NB_PARTICULES; j++)
    {
        float dist_sqr = vec2_dist_sqrd(positions[i], positions[j]);
        if (dist_sqr < 4 * s->radius * s->radius && dist_sqr > 0.0001)
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

            Vec2 d2 = vec2_mul_scalar(dir, (2 * s->radius - dist) / dist);

            vec2_add_inplace(positions + i, vec2_mul_scalar(d2, 0.5f));
            vec2_sub_inplace(positions + j, vec2_mul_scalar(d2, 0.5f));
        }
    }

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICULES; i++)
    {
        s->particules[i].pos = positions[i];
        vec2_add_inplace(&s->particules[i].velo, velocities[i]);
    }
}
