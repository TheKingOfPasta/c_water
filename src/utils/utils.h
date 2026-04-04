#pragma once

#include <stdio.h>

float randf(void);

char* read_all_file(char *file);

#define CLAMP(x, min, max) (x)<(min) ? (min) : (x)>(max) ? (max) : (x)

#define CLAMP01(x) CLAMP((x), 0, 1)

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define LOOP_NEIGHBOURS(pos, radius)\
    for (int cx = (((int)(pos).x) / s->chunk_size < 0 ? 0 : (int)(pos).x / s->chunk_size >= s->nb_chunk_x ? s->nb_chunk_x - 1 : (int)(pos).x / s->chunk_size),\
             cy = (((int)(pos).y) / s->chunk_size < 0 ? 0 : (int)(pos).y / s->chunk_size >= s->nb_chunk_y ? s->nb_chunk_y - 1 : (int)(pos).y / s->chunk_size),\
             _done = 0; !_done; _done = 1)\
    for (int dx = -(radius); dx <= (radius); dx++)\
        for (int dy = -(radius); dy <= (radius); dy++)\
            if (!(cx + dx < 0 || cy + dy < 0 || cx + dx >= s->nb_chunk_x || cy + dy >= s->nb_chunk_y))\
                for (int nx = cx + dx, ny = cy + dy, start = s->start_chunk[nx + ny * s->nb_chunk_x], end = s->end_chunk[nx + ny * s->nb_chunk_x], i = start; i < end; i++)
