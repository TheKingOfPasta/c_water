#include "utils.h"

#include <assert.h>
#include <stdlib.h>

float randf(void)
{
    return (float)(rand()) / RAND_MAX;
}

char* read_all_file(char *file)
{
    FILE* f = fopen(file, "r");
    assert(f != NULL);
    fseek(f, 0, SEEK_END);
    int eof = ftell(f);
    rewind(f);

    char* res = calloc(eof + 1, sizeof(char));

    fread(res, eof, sizeof(char), f);

    fclose(f);

    return res;
}
