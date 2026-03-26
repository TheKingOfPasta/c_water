#pragma once

typedef struct
{
    float x;
    float y;
} Vec2;

Vec2 vec2_zero(void);
Vec2 vec2_ones(void);
Vec2 vec2_random(void);

Vec2 vec2_add(Vec2 a, Vec2 b);
void vec2_add_inplace(Vec2* a, Vec2 b);

Vec2 vec2_sub(Vec2 a, Vec2 b);
void vec2_sub_inplace(Vec2* a, Vec2 b);

Vec2 vec2_neg(Vec2 v);

Vec2 vec2_mul_scalar(Vec2 v, float s);

float vec2_dot(Vec2 a, Vec2 b);

float vec2_norm_sqrd(Vec2 v);
float vec2_norm(Vec2 v);
Vec2 vec2_normalized(Vec2 v);

float vec2_dist_sqrd(Vec2 a, Vec2 b);
float vec2_dist(Vec2 a, Vec2 b);

bool vec2_equal(Vec2 a, Vec2 b);

void vec2_print(const Vec2* v);
