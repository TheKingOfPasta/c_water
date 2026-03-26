#include "colorRGB8.h"

RGB8 rgb8_cyan(void)
{
    return (RGB8){ 0, 255, 255 };
}

RGB8 rgb8_red(void)
{
    return (RGB8){ 255, 0, 0 };
}

RGB8 rgb8_white(void)
{
    return (RGB8){ 255, 255, 255 };
}

RGB8 rgb8_black(void)
{
    return (RGB8){ 0, 0, 0 };
}

RGB8 rgb8_blue(void)
{
    return (RGB8){ 0, 0, 255 };
}

void rgb8_print(const RGB8* c)
{
    printf("( %d %d %d )", c->r, c->g, c->b);
}

inline uint8_t clamp_u8(int v)
{
    if (v < 0)
        return 0;
    if (v > 255)
        return 255;
    return (uint8_t)v;
}

inline RGB8 rgb8_lerp(RGB8 a, RGB8 b, float y)
{
    return (RGB8){ (uint8_t)(a.r * (1.0f - y) + b.r * y),
                   (uint8_t)(a.g * (1.0f - y) + b.g * y),
                   (uint8_t)(a.b * (1.0f - y) + b.b * y) };
}

inline float rgb8_norm(RGB8 c)
{
    return (float)(c.r + c.g + c.b);
}

inline RGB8 rgb8_add(RGB8 a, RGB8 b)
{
    return (RGB8){ clamp_u8(a.r + b.r), clamp_u8(a.g + b.g),
                   clamp_u8(a.b + b.b) };
}

inline RGB8 rgb8_mul(RGB8 c, float l)
{
    return (RGB8){ (uint8_t)(c.r * l), (uint8_t)(c.g * l), (uint8_t)(c.b * l) };
}
