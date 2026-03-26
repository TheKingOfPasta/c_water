#pragma once

float randf(void);

#define CLAMP(x, min, max) (x)<(min) ? (min) : (x)>(max) ? (max) : (x)

#define CLAMP01(x) CLAMP((x), 0, 1)
