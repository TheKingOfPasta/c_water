#include "simulation.h"

#include <assert.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>

#include "config_reloader.h"
#include "utils/vec2.h"

Simulation simulation_gen()
{
    Simulation s = { 0 };

    int pts = sqrt(NB_PARTICLES);
    for (int i = 0; i < pts; i++)
    for (int j = 0; j < pts; j++)
    {
        s.particles[i + j * pts].velo = vec2_zero();
        s.particles[i + j * pts].pos = (Vec2){ .x = (c->radius + 1) * i + c->sx / 2, .y = (c->radius + 1) * j + c->sy / 2 };
    }

    s.chunk_size = c->radius * CHUNK_SIZE_SCALE_COMPARED_TO_PARTICLE_RADIUS;
    s.nb_chunk_x = c->sx / s.chunk_size + 1;
    s.nb_chunk_y = c->sy / s.chunk_size + 1;

    s.start_chunk = malloc(sizeof(uint32_t) * s.nb_chunk_x * s.nb_chunk_y);
    s.end_chunk = malloc(sizeof(uint32_t) * s.nb_chunk_x * s.nb_chunk_y);

    return s;
}

void simulation_free(Simulation* s)
{
    free(s->start_chunk);
    s->start_chunk = NULL;
    free(s->end_chunk);
    s->end_chunk = NULL;
}

int pair_sort(const void* p1, const void* p2)
{
    uint16_t a = ((chunk_particle_idx_pair*)p1)->chunk_idx;
    uint16_t b = ((chunk_particle_idx_pair*)p2)->chunk_idx;
    return a - b;
}

uint16_t particle_get_chunk_idx(Particle* p, Simulation* s)
{
    int cx = ((int)p->pos.x) / s->chunk_size;
    int cy = ((int)p->pos.y) / s->chunk_size;

    if (cx < 0)
        cx = 0;
    if (cy < 0)
        cy = 0;
    if (cx >= s->nb_chunk_x)
        cx = s->nb_chunk_x - 1;
    if (cy >= s->nb_chunk_y)
        cy = s->nb_chunk_y - 1;

    return cy * s->nb_chunk_x + cx;
}

void simulation_update_chunks(Simulation* s)
{
    for (int i = 0; i < NB_PARTICLES; i++)
    {
        s->pairs[i].chunk_idx = particle_get_chunk_idx(s->particles + i, s);
        s->pairs[i].particle_idx = i;
    }

    qsort(s->pairs, NB_PARTICLES, sizeof(chunk_particle_idx_pair), pair_sort);

    for (int i = 0; i < s->nb_chunk_x * s->nb_chunk_y; i++)
    {
        s->start_chunk[i] = CHUNK_EMPTY_IDX;
        s->end_chunk[i] = CHUNK_EMPTY_IDX;
    }

    uint16_t last_idx = CHUNK_EMPTY_IDX;
    for (int i = 0; i < NB_PARTICLES; i++)
    {
        uint16_t curr_idx = s->pairs[i].chunk_idx;
        if (last_idx != curr_idx)
        {
            if (last_idx != CHUNK_EMPTY_IDX)
            {
                s->end_chunk[last_idx] = i;
            }
            last_idx = curr_idx;
            s->start_chunk[last_idx] = i;
        }
    }
    if (last_idx != CHUNK_EMPTY_IDX)
    {
        s->end_chunk[last_idx] = NB_PARTICLES;
    }
}

float simulation_compute_density(Simulation* s, Particle* p)
{
    const float mass = 1.0f;

    float d = 0.0f;

    const int chunk_check_radius = c->particle_influence_radius / s->chunk_size + 1;

    int cx = ((int)p->pos.x) / s->chunk_size;
    int cy = ((int)p->pos.y) / s->chunk_size;

    for (int dx = -chunk_check_radius; dx <= chunk_check_radius; dx++)
    {
        for (int dy = -chunk_check_radius; dy <= chunk_check_radius; dy++)
        {
            int nx = cx + dx;
            int ny = cy + dy;

            if (nx < 0 || ny < 0 || nx >= s->nb_chunk_x || ny >= s->nb_chunk_y)
                continue;

            int chunk_idx = nx + ny * s->nb_chunk_x;

            int start = s->start_chunk[chunk_idx];
            int end = s->end_chunk[chunk_idx];

            for (int i = start; i < end; i++)
            {
                Particle* other = &s->particles[s->pairs[i].particle_idx];

                float dist = vec2_dist(p->pos, other->pos);
                d += particle_density(dist);
            }
        }
    }
    /*for (int k = 0; k < NB_PARTICLES; k++)
    {
        d += particle_density(vec2_dist(s->particles[k].pos, p->pos));
    }*/

    return d * mass;
}

static void simulation_update_density_field(Simulation* s)
{
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;
        float d = simulation_compute_density(s, p);

        s->particle_densities[i] = d;
    }
}

void simulation_step(Simulation* s)
{
    simulation_update_chunks(s);

    simulation_update_density_field(s);

    Vec2 predicted_positions[NB_PARTICLES] = { 0 };

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;

        vec2_add_inplace(&p->velo, (Vec2){ .x = 0, .y = 0.0981f * c->gravity_multiplier * s->dt });

        predicted_positions[i] = p->pos;
        vec2_add_inplace(predicted_positions + i, p->velo);
    }

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;

        Vec2 grad = particle_compute_pressure(s, predicted_positions, i);

        grad = vec2_neg(grad);

        grad = vec2_mul_scalar(grad, s->dt * c->pressure_force / s->particle_densities[i]);

        vec2_add_inplace(&p->velo, grad);

        vec2_add_inplace(&p->pos, p->velo);

        p->velo = vec2_mul_scalar(p->velo, c->velocity_drag);

        particle_interact_bounds(p);
    }

    Vec2 velocities[NB_PARTICLES] = { 0 };

    Vec2 positions[NB_PARTICLES];
#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
        positions[i] = s->particles[i].pos;

    float rad4 = 4 * c->radius * c->radius;

    const int chunk_check_radius = 1;

#pragma omp parallel for
    for (int i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;

        int cx = ((int)p->pos.x) / s->chunk_size;
        int cy = ((int)p->pos.y) / s->chunk_size;

        for (int dx = -chunk_check_radius; dx <= chunk_check_radius; dx++)
        {
            for (int dy = -chunk_check_radius; dy <= chunk_check_radius; dy++)
            {
                int nx = cx + dx;
                int ny = cy + dy;

                if (nx < 0 || ny < 0 || nx >= s->nb_chunk_x || ny >= s->nb_chunk_y)
                    continue;

                int chunk_idx = nx + ny * s->nb_chunk_x;

                int start = s->start_chunk[chunk_idx];
                int end = s->end_chunk[chunk_idx];

                for (int j = start; j < end; j++)
                {
                    int index = s->pairs[j].particle_idx;
                    if (index < i)
                        continue;

                    float dist_sqr = vec2_dist_sqrd(positions[i], positions[index]);
                    if (dist_sqr < rad4 && dist_sqr > 0.0001)
                    {
                        float dist = sqrtf(dist_sqr);

                        Vec2 dir = vec2_sub(positions[i], positions[index]);
                        float dot = vec2_dot(
                            vec2_sub(s->particles[i].velo, s->particles[index].velo),
                            dir);

                        if (dot < 0)
                        {
                            Vec2 v_diff = vec2_mul_scalar(dir, dot / dist_sqr);

                            v_diff =
                                vec2_mul_scalar(v_diff, c->velocity_collision_dampner);

                            vec2_sub_inplace(velocities + i, v_diff);
                            vec2_add_inplace(velocities + index, v_diff);
                        }

                        Vec2 d2 = vec2_mul_scalar(dir, (2 * c->radius - dist) / dist);

                        vec2_add_inplace(positions + i, vec2_mul_scalar(d2, 0.5f));
                        vec2_sub_inplace(positions + index, vec2_mul_scalar(d2, 0.5f));
                    }
                }
            }
        }
    }

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        s->particles[i].pos = positions[i];
        vec2_add_inplace(&s->particles[i].velo, velocities[i]);
    }
}
