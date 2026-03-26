#include "particule.h"

#include <math.h>
#include <stdio.h>

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
        p->pos.x = -p->pos.x;
        if (p->pos.x > sx)
            p->pos.x = 0;
    }
    else if (p->pos.x >= sx)
    {
        p->velo.x *= -1;
        p->pos.x = sx - (p->pos.x - sx);
        if (p->pos.x < 0)
            p->pos.x = sx;
    }

    if (p->pos.y < 0)
    {
        p->velo.y *= -1;
        p->pos.y = -p->pos.y;
        if (p->pos.y > sy)
            p->pos.y = 0;
    }
    else if (p->pos.y >= sy)
    {
        p->velo.y *= -1;
        p->pos.y = sy - (p->pos.y - sy);
        if (p->pos.y < 0)
            p->pos.y = sy;
    }
}

static inline void particule_apply_gravity(Particule* p)
{
    vec2_add_inplace(&p->velo, (Vec2){ 0, 0.5 });
}

void particule_step(Particule* p, int sx, int sy)
{
    vec2_add_inplace(&p->pos, p->velo);

    particule_interact_bounds(p, sx, sy);
    //    particule_apply_gravity(p);
}

float particule_density(Particule* p, Vec2 sample)
{
    const float vol = 6.0 / (M_PI * pow(PARTICULE_RADIUS, 4));

    float d = vec2_dist(p->pos, sample);

    if (d > PARTICULE_RADIUS)
        return 0;

    float v = PARTICULE_RADIUS - d;
    return v * v * vol;
}
