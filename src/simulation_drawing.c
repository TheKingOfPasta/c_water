#include <float.h>

#include "image_drawing.h"
#include "simulation.h"

void simulation_draw_balls(Simulation* s, Image* img)
{
    const RGB8 circle_color = (RGB8){ .r = 10, .g = 255, .b = 255 };

    for (int i = 0; i < NB_PARTICULES; i++)
    {
        Vec2 p = s->particules[i].pos;

        image_draw_circle(img, p.x, p.y, s->radius, circle_color);
    }

    const int bb[4][2] = {
        { 0, 0 },
        { s->sx - 1, 0 },
        { s->sx - 1, s->sy - 1 },
        { 0, s->sy - 1 },
    };

    for (int i = 0; i < 4; i++)
    {
        image_draw_line(img, bb[i][0], bb[i][1], bb[(i + 1) % 4][0],
                        bb[(i + 1) % 4][1], rgb8_white());
    }
}

void simulation_draw_field(Simulation* s, Image* img)
{
    return;
    float max_density = -1000000.0f;
    float min_density = FLT_MAX;
    for (int i = 0; i < s->sx * s->sy; i++)
    {
        if (max_density < s->density_field[i])
            max_density = s->density_field[i];
        if (min_density > s->density_field[i])
            min_density = s->density_field[i];
    }

    float density_diff = max_density - min_density;
    for (int i = 0; i < s->sx; i++)
        for (int j = 0; j < s->sy; j++)
        {
            uint8_t d = (s->density_field[i + j * s->sx] - min_density)
                / density_diff * 255;
            image_set_color(img, i, j, (RGB8){ d, d, d });
        }
}
