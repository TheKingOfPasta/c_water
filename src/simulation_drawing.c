#include <math.h>
#include <stdlib.h>

#include "image_drawing.h"
#include "simulation.h"

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

    float max_norm = 0.001;

    for (int i = 0; i < gradients_size; i++)
    {
        float n = vec2_norm_sqrd(gradients[i]);
        if (max_norm < n)
            max_norm = n;
    }
    max_norm = sqrt(max_norm);

    for (int x = 0; x < nb_arrow_x; x++)
        for (int y = 0; y < nb_arrow_y; y++)
        {
            int xo = padding + x * padding_arr + padding_arr / 2;
            int yo = padding + y * padding_arr + padding_arr / 2;

            Vec2 g = gradients[x + y * nb_arrow_x];

            // image_draw_circle(img, xo, yo, 2, rgb8_cyan());
            image_draw_vector(img, xo, yo, xo + g.x / max_norm * padding_arr,
                              yo + g.y / max_norm * padding_arr, rgb8_red());
        }

    free(gradients);
}

void simulation_draw_mouse_gradient(Simulation* s, Image* img, int padding,
                                    int mouse_x, int mouse_y)
{
    int sx = mouse_x - padding;
    int sy = mouse_y - padding;

    if (sx < 0 || sy < 0 || sx >= s->sx || sy >= s->sy)
        return;

    Vec2 g = simulation_compute_gradient(s, sx, sy);

    image_draw_circle(img, mouse_x, mouse_y, 4, rgb8_cyan());

    const float display_len = 40.0f;
    float norm = sqrtf(vec2_norm_sqrd(g));
    if (norm < 0.000001f)
        return;

    image_draw_vector(img, mouse_x, mouse_y,
                      mouse_x + (int)(g.x / norm * display_len),
                      mouse_y + (int)(g.y / norm * display_len), rgb8_cyan());
}
