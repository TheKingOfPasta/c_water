#include "utils.h"

#include <stdlib.h>

float randf(void)
{
    return (float)(rand()) / RAND_MAX;
}

char* read_all_file(FILE* f)
{
    fseek(f, 0, SEEK_END);
    int eof = ftell(f);
    rewind(f);

    char* res = calloc(eof + 1, sizeof(char));

    fread(res, eof, sizeof(char), f);

    return res;
}
