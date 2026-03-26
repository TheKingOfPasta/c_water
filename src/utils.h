#pragma once

#include <stdio.h>

float randf(void);

char* read_all_file(FILE* f);

#define CLAMP(x, min, max) (x)<(min) ? (min) : (x)>(max) ? (max) : (x)

#define CLAMP01(x) CLAMP((x), 0, 1)
