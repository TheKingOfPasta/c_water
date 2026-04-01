#include "image_drawing.h"

#include <stdlib.h>

#include "image.h"
#include "utils/vec2.h"

void image_draw_circle(Image* i, int cx, int cy, int r, RGB8 c)
{
    for (int y = cy - r; y <= cy + r; y++)
        for (int x = cx - r; x <= cx + r; x++)
        {
            if (!image_in_bounds(i, x, y))
                continue;

            int dx = x - cx;
            int dy = y - cy;

            if (dx * dx + dy * dy <= r * r)
            {
                image_set_color(i, x, y, c);
            }
        }
}

void image_draw_line(Image* i, int x0, int y0, int x1, int y1, RGB8 c)
{
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (1)
    {
        if (image_in_bounds(i, x0, y0))
        {
            image_set_color(i, x0, y0, c);
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void image_draw_vector(Image* i, int x0, int y0, int x1, int y1, RGB8 c)
{
    const int size_arrow = 10;

    image_draw_line(i, x0, y0, x1, y1, c);
    Vec2 dir = vec2_normalized((Vec2){ x1 - x0, y1 - y0 });
    Vec2 orth = (Vec2){ dir.y, -dir.x };

    Vec2 midpoint =
        vec2_add((Vec2){ x1, y1 }, vec2_mul_scalar(dir, -size_arrow));

    Vec2 left_wing = vec2_add(midpoint, vec2_mul_scalar(orth, -size_arrow));
    Vec2 right_wing = vec2_add(midpoint, vec2_mul_scalar(orth, size_arrow));

    image_draw_line(i, left_wing.x, left_wing.y, x1, y1, c);
    image_draw_line(i, right_wing.x, right_wing.y, x1, y1, c);
}

void image_draw_square_alligned(Image* img, int x, int y, int sx, int sy,
                                RGB8 c)
{
    const int bb[4][2] = {
        { x, y },
        { x + sx, y },
        { x + sx, y + sy },
        { x, y + sy },
    };

    for (int i = 0; i < 4; i++)
    {
        image_draw_line(img, bb[i][0], bb[i][1], bb[(i + 1) % 4][0],
                        bb[(i + 1) % 4][1], c);
    }
}
