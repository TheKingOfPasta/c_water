#include <float.h>
#include <stdio.h>

#include "colorRGB8.h"
#include "image_drawing.h"
#include "simulation.h"
#include "vec2.h"

void simulation_draw_border(Simulation* s, Image* img)
{
    image_draw_square_alligned(img, 0, 0, s->sx - 1, s->sy - 1, rgb8_white());
}

void simulation_draw_balls(Simulation* s, Image* img)
{
    const RGB8 circle_color = (RGB8){ .r = 10, .g = 255, .b = 255 };

    for (int i = 0; i < NB_PARTICULES; i++)
    {
        Vec2 p = s->particules[i].pos;

        image_draw_circle(img, p.x, p.y, s->radius, circle_color);
    }
}

void simulation_draw_field([[maybe_unused]] Simulation* s,
                           [[maybe_unused]] Image* img)
{
    return;
}

void simulation_draw_chunks(Simulation* s, Image* img, float x, float y)
{
    simulation_draw_balls(s, img);
    simulation_update_chunks(s);

    int cx = x / s->chunk_size;
    int cy = y / s->chunk_size;

    if (cx >= 0 && cy >= 0 && cx < s->nb_chunk_x && cy < s->nb_chunk_y)
    {
        // printf("\n\nx:%2d  y:%2d     cx:%2d cy:%2d     chunksize:%d\n", cx,
        // cy,
        //        s->nb_chunk_x, s->nb_chunk_y, CHUNK_SIZE);
        int chunk_idx = cx + cy * s->nb_chunk_x;
        int start_idx = s->start_chunk[chunk_idx];
        int end_idx = s->end_chunk[chunk_idx];
        for (int i = start_idx; i < end_idx; i++)
        {
            Vec2 p = s->particules[s->pairs[i].particle_idx].pos;

            //     vec2_print(&p);
            //     printf("\n");
            image_draw_circle(img, p.x, p.y, s->radius,
                              (RGB8){ .r = 250, .b = 0, .g = 0 });
        }
    }

    const RGB8 chunk_border = (RGB8){ .r = 140, .g = 140, .b = 140 };

    for (int i = 0; i < s->sx; i += s->chunk_size)
    {
        image_draw_line(img, i, 0, i, s->sy, chunk_border);
    }

    for (int i = 0; i < s->sy; i += s->chunk_size)
    {
        image_draw_line(img, 0, i, s->sx, i, chunk_border);
    }

    if (cx >= 0 && cy >= 0 && cx < s->nb_chunk_x && cy < s->nb_chunk_y)
    {
        image_draw_square_alligned(img, cx * s->chunk_size, cy * s->chunk_size,
                                   s->chunk_size, s->chunk_size, rgb8_white());
    }
}

void simulation_print_chunks(Simulation* s)
{
    printf("nb_chunk_x = %d\n", s->nb_chunk_x);
    printf("nb_chunk_y = %d\n", s->nb_chunk_y);

    printf("pairs = [\n");
    for (int i = 0; i < NB_PARTICULES; i++)
    {
        printf("    [%d] = %d - %d   \n", i, s->pairs[i].chunk_idx,
               s->pairs[i].particle_idx);
    }

    printf("]\n start_idx =  [\n");
    for (int i = 0; i < s->nb_chunk_x * s->nb_chunk_y; i++)
    {
        int idx = s->start_chunk[i];
        if (idx != CHUNK_EMPTY_IDX)
        {
            printf("[%d] = %d\n", i, idx);
        }
    }

    printf("]\n end_idx =  [\n");
    for (int i = 0; i < s->nb_chunk_x * s->nb_chunk_y; i++)
    {
        int idx = s->end_chunk[i];
        if (idx != CHUNK_EMPTY_IDX)
        {
            printf("[%d] = %d\n", i, idx);
        }
    }
}
