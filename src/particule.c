#include "particule.h"

#include <stdio.h>

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

static inline void particule_apply_gravity(Particule* p)
{
    vec2_add_inplace(&p->velo, (Vec2){ 0, 0.5 });
}

void particule_step(Particule* p, int sx, int sy)
{
    vec2_add_inplace(&p->pos, p->velo);

    if (p->pos.x < 0)
    {
        p->velo.x *= -1;
        p->pos.x = -p->pos.x;
        if (p->pos.x > sx)
            p->pos.x = 0;
    }

    if (p->pos.y < 0)
    {
        p->velo.y *= -1;
        p->pos.y = -p->pos.y;
        if (p->pos.y > sy)
            p->pos.y = 0;
    }

    if (p->pos.x >= sx)
    {
        p->velo.x *= -1;
        p->pos.x = sx - (p->pos.x - sx);
        if (p->pos.x < 0)
            p->pos.x = sx;
    }

    if (p->pos.y >= sy)
    {
        p->velo.y *= -1;
        p->pos.y = sy - (p->pos.y - sy);
        if (p->pos.y < 0)
            p->pos.y = sy;
    }

    //    particule_apply_gravity(p);
}

float particule_density(Particule* p, Vec2 sample)
{
    float d = vec2_dist(p->pos, sample);

    if (PARTICULE_RADIUS < d)
        return 0;

    float v = PARTICULE_RADIUS - d;
    return v * v;
}
