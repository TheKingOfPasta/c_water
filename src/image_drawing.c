#include "image_drawing.h"

#include "image.h"
#include "stdlib.h"

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
