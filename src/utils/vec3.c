#include "vec3.h"

#include <math.h>
#include <stdio.h>

Vec3 vec3_zero(void)
{
    return (Vec3){ 0.0f, 0.0f, 0.0f };
}

Vec3 vec3_ones(void)
{
    return (Vec3){ 1.0f, 1.0f, 1.0f };
}

Vec3 vec3_up(void)
{
    return (Vec3){ 0.0f, 1.0f, 0.0f };
}

Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

void vec3_add_inplace(Vec3* a, Vec3 b)
{
    a->x += b.x;
    a->y += b.y;
    a->z += b.z;
}

Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

void vec3_sub_inplace(Vec3* a, Vec3 b)
{
    a->x -= b.x;
    a->y -= b.y;
    a->z -= b.z;
}

Vec3 vec3_neg(Vec3 v)
{
    return (Vec3){ -v.x, -v.y, -v.z };
}

Vec3 vec3_mul_scalar(Vec3 v, float s)
{
    return (Vec3){ v.x * s, v.y * s, v.z * s };
}

float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return (Vec3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

float vec3_norm_sqrd(Vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vec3_norm(Vec3 v)
{
    return sqrtf(vec3_norm_sqrd(v));
}

Vec3 vec3_normalized(Vec3 v)
{
    float n = vec3_norm(v);
    if (n == 0.0f)
        return vec3_zero();
    return vec3_mul_scalar(v, 1.0f / n);
}

float vec3_dist_sqrd(Vec3 a, Vec3 b)
{
    return vec3_norm_sqrd(vec3_sub(b, a));
}

Vec3 vec3_reflect(Vec3 I, Vec3 N)
{
    return vec3_sub(I, vec3_mul_scalar(N, 2.0f * vec3_dot(I, N)));
}

bool vec3_equal(Vec3 a, Vec3 b)
{
    const float eps = 0.003f;
    return fabsf(a.x - b.x) < eps && fabsf(a.y - b.y) < eps && fabsf(a.z - b.z) < eps;
}

void vec3_print(const Vec3* v)
{
    printf("( %f %f %f )", v->x, v->y, v->z);
}
