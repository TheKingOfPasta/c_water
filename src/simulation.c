#include "simulation.h"

#include "image_drawing.h"

Simulation simulation_gen(int sx, int sy)
{
    Simulation res = {
        .sx = sx,
        .sy = sy,
    };

    for (int i = 0; i < NB_POINTS; i++)
    {
        res.points[i] = vec2_random();
        res.points[i].x *= sx;
        res.points[i].y *= sy;
    }

    return res;
}

void simulation_draw(Simulation* s, Image* img)
{
    const RGB8 circle_color = (RGB8){ .r = 40, .g = 40, .b = 150 };

    const int padding = 30;

    const int sx_pad = img->sx - padding * 2;
    const int sy_pad = img->sy - padding * 2;

    for (int i = 0; i < NB_POINTS; i++)
    {
        Vec2 p = s->points[i];

        image_draw_circle(img, padding + p.x / s->sx * sx_pad,
                          padding + p.y / s->sy * sy_pad, 2, circle_color);
    }

    const int bb[4][2] = {
        { padding, padding },
        { img->sx - padding, padding },
        { img->sx - padding, img->sy - padding },
        { padding, img->sy - padding },
    };

    for (int i = 0; i < 4; i++)
    {
        image_draw_line(img, bb[i][0], bb[i][1], bb[(i + 1) % 4][0],
                        bb[(i + 1) % 4][1], rgb8_white());
    }
}
