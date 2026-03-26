#include "simulation.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "colorRGB8.h"
#include "image_drawing.h"
#include "vec2.h"

Simulation simulation_gen(int sx, int sy)
{
    Simulation res = {
        .sx = sx,
        .sy = sy,
    };

    for (int i = 0; i < NB_PARTICULES; i++)
    {
        res.particules[i] = particule_gen_random(sx, sy);
    }

    res.density_field = calloc(sx * sy, sizeof(*res.density_field));
    return res;
}

void simulation_free(Simulation* s)
{
    free(s->density_field);
}

void simulation_draw_balls(Simulation* s, Image* img, int padding)
{
    const int particule_radius = 5;
    const RGB8 circle_color = (RGB8){ .r = 40, .g = 40, .b = 150 };

    for (int i = 0; i < NB_PARTICULES; i++)
    {
        Vec2 p = s->particules[i].pos;

        image_draw_circle(img, padding + p.x, padding + p.y, particule_radius,
                          circle_color);
    }

    const int bb[4][2] = {
        { padding, padding },
        { s->sx + padding, padding },
        { s->sx + padding, s->sy + padding },
        { padding, s->sy + padding },
    };

    for (int i = 0; i < 4; i++)
    {
        image_draw_line(img, bb[i][0], bb[i][1], bb[(i + 1) % 4][0],
                        bb[(i + 1) % 4][1], rgb8_white());
    }
}

static void simulation_update_field(Simulation* s)
{
    for (int i = 0; i < s->sx * s->sy; i++)
    {
        float d = 0;
        for (int j = 0; j < NB_PARTICULES; j++)
        {
            d += particule_density(&s->particules[j],
                                   (Vec2){
                                       i % s->sx,
                                       (int)(i / s->sx),
                                   });
        }
        s->density_field[i] = d;
    }
}

void simulation_draw_field(Simulation* s, Image* img, int padding)
{
    float max_density = 0.1f;
    for (int i = 0; i < s->sx * s->sy; i++)
    {
        if (max_density < s->density_field[i])
            max_density = s->density_field[i];
    }

    for (int i = 0; i < s->sx * s->sy; i++)
    {
        uint8_t d = s->density_field[i] / max_density * 255;
        image_set_color(img, i % s->sx + padding, i / s->sx + padding,
                        (RGB8){ d, d, d });
    }
}

Vec2 simulation_compute_gradient(Simulation* s, int x, int y)
{
    int xm = (x > 0) ? x - 1 : x;
    int xp = (x < s->sx - 1) ? x + 1 : x;
    int ym = (y > 0) ? y - 1 : y;
    int yp = (y < s->sy - 1) ? y + 1 : y;

    float d_xm = s->density_field[ym * s->sx + xm];
    float d_xp = s->density_field[ym * s->sx + xp];
    float d_ym = s->density_field[ym * s->sx + x];
    float d_yp = s->density_field[yp * s->sx + x];

    return vec2_mul_scalar((Vec2){ d_xp - d_xm, d_yp - d_ym }, 0.5f);
}

void simulation_draw_field_arrow(Simulation* s, Image* img, int padding)
{
    const int number_arrow = 15;
    const int padding_arr = (int)(s->sx / number_arrow);

    const int nb_arrow_x = s->sx / padding_arr;
    const int nb_arrow_y = s->sy / padding_arr;

    Vec2* gradients = calloc(nb_arrow_x * nb_arrow_y, sizeof(Vec2));
    int gradients_size = 0;

    for (int x = 0; x < nb_arrow_x; x++)
        for (int y = 0; y < nb_arrow_y; y++)
        {
            gradients[gradients_size++] = simulation_compute_gradient(
                s, x * padding_arr + padding_arr / 2,
                y * padding_arr + padding_arr / 2);
        }

    float mx = 0.1;
    float my = 0.1;

    for (int i = 0; i < gradients_size; i++)
    {
        if (gradients[i].x > mx)
            mx = gradients[i].x;
        if (gradients[i].y > my)
            my = gradients[i].y;
    }

    for (int x = 0; x < nb_arrow_x; x++)
        for (int y = 0; y < nb_arrow_y; y++)
        {
            int xo = padding + x * padding_arr + padding_arr / 2;
            int yo = padding + y * padding_arr + padding_arr / 2;

            Vec2 g = gradients[x + y * nb_arrow_x];

            // image_draw_circle(img, xo, yo, 2, rgb8_cyan());
            image_draw_vector(img, xo, yo, xo + (g.x) / mx * padding_arr / 2,
                              yo + (g.y) / my * padding_arr / 2, rgb8_red());
        }

    free(gradients);
}

void simulation_step(Simulation* s)
{
    simulation_update_field(s);
    for (int i = 0; i < NB_PARTICULES; i++)
    {
        particule_step(&s->particules[i], s->sx, s->sy);
    }
}
