#include "image.h"

#include <stdlib.h>

#include "colorRGB8.h"

Image image_blank(int sx, int sy)
{
    RGB8* pxls = calloc(sx * sy, sizeof(RGB8));
    return (Image){ sx, sy, pxls };
}

void image_set_color(Image* i, int x, int y, RGB8 c)
{
    i->pixels[y * i->sx + x] = c;
}
void image_fill(Image* i, RGB8* c)
{
    for (int x = 0; x < i->sx; x++)
        for (int y = 0; y < i->sy; y++)
            i->pixels[y * i->sx + x] = *c;
}

bool image_in_bounds(Image* i, int x, int y)
{
    return x >= 0 && x < i->sx && y >= 0 && y < i->sy;
}

void image_savePPM(const Image* i, const char* filename)
{
    FILE* f = fopen(filename, "wb");
    if (!f)
    {
        fprintf(stderr, "Failed to open file %s", filename);
        return;
    }

    fprintf(f, "P6\n%u %u\n255\n", i->sx, i->sy);

    size_t size = i->sx * i->sy * sizeof(RGB8);
    fwrite(i->pixels, 1, size, f);

    fclose(f);
}
