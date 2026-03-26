#pragma once

#include <stdio.h>

float randf(void);

char* read_all_file(FILE* f);

#define CLAMP(x, min, max) (x)<(min) ? (min) : (x)>(max) ? (max) : (x)

#define CLAMP01(x) CLAMP((x), 0, 1)

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif
