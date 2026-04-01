#pragma once

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB8;

RGB8 rgb8_cyan(void);

RGB8 rgb8_red(void);

RGB8 rgb8_white(void);

RGB8 rgb8_black(void);

RGB8 rgb8_blue(void);

void rgb8_print(const RGB8* c);

RGB8 rgb8_lerp(RGB8 a, RGB8 b, float y);

float rgb8_norm(RGB8 c);

RGB8 rgb8_add(RGB8 a, RGB8 b);

RGB8 rgb8_mul(RGB8 c, float l);
