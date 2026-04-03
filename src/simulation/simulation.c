#include "simulation.h"

#include <assert.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>

#include "config_reloader.h"
#include "utils/utils.h"
#include "utils/vec2.h"

Simulation simulation_gen()
{
    Simulation s = { 0 };

    int pts_x = (int)ceil(sqrt(NB_PARTICLES));
    int pts_y = (NB_PARTICLES + pts_x - 1) / pts_x;

    float padding = 2.0f * c->radius + 1.0f;

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * c->radius * 0.3f;
        float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * c->radius * 0.3f;

        s.particles[i].velo = vec2_zero();

        s.particles[i].pos = (Vec2){
            .x = c->sx / 2.0f + ((i % pts_x) - pts_x / 2.0f) * padding + rx,
            .y = c->sy / 2.0f + ((int)(i / pts_y) - pts_y / 2.0f) * padding + ry
        };
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

uint16_t particle_get_chunk_idx(Vec2* pos, Simulation* s)
{
    int cx = ((int)pos->x) / s->chunk_size;
    int cy = ((int)pos->y) / s->chunk_size;

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

static inline void simulation_update_chunks(Simulation* s, Vec2 predicted_positions[NB_PARTICLES])
{
    for (int i = 0; i < NB_PARTICLES; i++)
    {
        s->pairs[i].chunk_idx = particle_get_chunk_idx(predicted_positions + i, s);
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

float simulation_compute_density(Simulation* s, Vec2 predicted_positions[NB_PARTICLES], size_t index)
{
    const float mass = 1.0f;

    float d = 0.0f;

    LOOP_NEIGHBOURS(predicted_positions[index], c->particle_influence_radius / s->chunk_size + 1)
    {
        float dist = vec2_dist(predicted_positions[index], predicted_positions[s->pairs[i].particle_idx]);
        d += particle_density(dist);
    }

    /*for (int k = 0; k < NB_PARTICLES; k++)
    {
        d += particle_density(vec2_dist(s->particles[k].pos, p->pos));
    }*/

    return d * mass;
}

static void simulation_update_density_field(Simulation* s, Vec2 predicted_positions[NB_PARTICLES])
{
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        float d = simulation_compute_density(s, predicted_positions, i);

        s->particle_densities[i] = d;
    }
}

void simulation_step(Simulation* s)
{
    Vec2 predicted_positions[NB_PARTICLES] = { 0 };

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;

        vec2_add_inplace(
            &p->velo,
            (Vec2){ .x = 0, .y = 0.0981f * c->gravity_multiplier * s->dt });

        predicted_positions[i] = p->pos;
        vec2_add_inplace(predicted_positions + i, p->velo);
    }

    simulation_update_chunks(s, predicted_positions);

    simulation_update_density_field(s, predicted_positions);

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;

        Vec2 grad = particle_compute_pressure(s, predicted_positions, i);

        grad = vec2_neg(grad);

        grad = vec2_mul_scalar(
            grad, s->dt * c->pressure_force / s->particle_densities[i]);

        vec2_add_inplace(&p->velo, grad);

        vec2_add_inplace(&p->pos, p->velo);

        p->velo = vec2_mul_scalar(p->velo, c->velocity_drag);

        particle_interact_bounds(p);
    }

    Vec2 velocities[NB_PARTICLES] = { 0 };
    bool collided[NB_PARTICLES] = { 0 };

    Vec2 positions[NB_PARTICLES];
#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
        positions[i] = s->particles[i].pos;

    float rad4 = 4 * c->radius * c->radius;

#pragma omp parallel for
    for (int j = 0; j < NB_PARTICLES; j++)
    {
        LOOP_NEIGHBOURS(s->particles[j].pos, 1)
        {
            int index = s->pairs[i].particle_idx;
            if (index < j)
                continue;

            float dist_sqrd =
                vec2_dist_sqrd(positions[j], positions[index]);
            if (dist_sqrd < rad4 && dist_sqrd > 0.0001)
            {
                float dist = sqrtf(dist_sqrd);

                Vec2 dir = vec2_sub(positions[j], positions[index]);
                float dot = vec2_dot(vec2_sub(s->particles[j].velo,
                                              s->particles[index].velo),
                                     dir);

                if (dot < 0)
                {
                    Vec2 v_diff = vec2_mul_scalar(dir, dot / dist_sqrd);

                    vec2_sub_inplace(velocities + j, v_diff);
                    vec2_add_inplace(velocities + index, v_diff);

                    collided[j] = true;
                    collided[index] = true;
                }

                Vec2 d2 =
                    vec2_mul_scalar(dir, (2 * c->radius - dist) / dist);

                vec2_add_inplace(positions + j,
                                 vec2_mul_scalar(d2, 0.5f));
                vec2_sub_inplace(positions + index,
                                 vec2_mul_scalar(d2, 0.5f));
            }
        }
    }

#pragma omp parallel for
    for (size_t i = 0; i < NB_PARTICLES; i++)
    {
        Particle* p = s->particles + i;

        p->pos = positions[i];
        vec2_add_inplace(&p->velo, velocities[i]);
        if (collided[i])
            p->velo = vec2_mul_scalar(p->velo, c->velocity_collision_dampner);
    }
}
