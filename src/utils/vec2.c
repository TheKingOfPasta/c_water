#include "vec2.h"

#include <math.h>
#include <stdio.h>

#include "utils.h"

Vec2 vec2_zero(void)
{
    return (Vec2){ 0.0f, 0.0f };
}

Vec2 vec2_ones(void)
{
    return (Vec2){ 1.0f, 1.0f };
}

Vec2 vec2_random(void)
{
    return (Vec2){ randf(), randf() };
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x + b.x, a.y + b.y };
}

void vec2_add_inplace(Vec2* a, Vec2 b)
{
    a->x += b.x;
    a->y += b.y;
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x - b.x, a.y - b.y };
}

void vec2_sub_inplace(Vec2* a, Vec2 b)
{
    a->x -= b.x;
    a->y -= b.y;
}

Vec2 vec2_neg(Vec2 v)
{
    return (Vec2){ -v.x, -v.y };
}

Vec2 vec2_mul_scalar(Vec2 v, float s)
{
    return (Vec2){ v.x * s, v.y * s };
}

float vec2_dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

float vec2_norm_sqrd(Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

float vec2_norm(Vec2 v)
{
    return sqrtf(vec2_norm_sqrd(v));
}

Vec2 vec2_normalized(Vec2 v)
{
    float n = vec2_norm(v);
    if (n == 0.0f)
        return vec2_zero();
    return vec2_mul_scalar(v, 1.0f / n);
}

float vec2_dist_sqrd(Vec2 a, Vec2 b)
{
    return vec2_norm_sqrd(vec2_sub(b, a));
}

float vec2_dist(Vec2 a, Vec2 b)
{
    return sqrt(vec2_dist_sqrd(a, b));
}

bool vec2_equal(Vec2 a, Vec2 b)
{
    const float eps = 0.003f;
    return fabsf(a.x - b.x) < eps && fabsf(a.y - b.y) < eps;
}

void vec2_print(const Vec2* v)
{
    printf("( %f %f )", v->x, v->y);
}
