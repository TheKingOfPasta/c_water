#pragma once

#include <stdbool.h>

typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

Vec3 vec3_zero(void);
Vec3 vec3_ones(void);
Vec3 vec3_up(void);

Vec3 vec3_add(Vec3 a, Vec3 b);
void vec3_add_inplace(Vec3* a, Vec3 b);

Vec3 vec3_sub(Vec3 a, Vec3 b);
void vec3_sub_inplace(Vec3* a, Vec3 b);

Vec3 vec3_neg(Vec3 v);

Vec3 vec3_mul_scalar(Vec3 v, float s);

float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);

float vec3_norm_sqrd(Vec3 v);
float vec3_norm(Vec3 v);
Vec3 vec3_normalized(Vec3 v);

float vec3_dist_sqrd(Vec3 a, Vec3 b);

Vec3 vec3_reflect(Vec3 I, Vec3 N);

bool vec3_equal(Vec3 a, Vec3 b);

void vec3_print(const Vec3* v);
