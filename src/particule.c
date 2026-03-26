#include "particule.h"

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

static void particule_apply_gravity(Particule* p)
{
    vec2_add_inplace(&p->velo, (Vec2){ 0, 0.5 });
}

void particule_step(Particule* p, int sx, int sy)
{
    vec2_add_inplace(&p->pos, p->velo);

    if (p->pos.x < 0 || p->pos.x >= sx)
    {
        p->velo.x *= -1;
        p->pos.x = CLAMP(p->pos.x, 0, sx);
    }

    if (p->pos.y < 0 || p->pos.y >= sy)
    {
        p->velo.y *= -1;
        p->pos.y = CLAMP(p->pos.y, 0, sy);
    }

    particule_apply_gravity(p);
}
