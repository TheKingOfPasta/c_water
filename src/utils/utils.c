#include "utils.h"

#include <assert.h>
#include <stdlib.h>

float randf(void)
{
    return (float)(rand()) / RAND_MAX;
}

char* read_all_file(char* file)
{
    FILE* f = fopen(file, "r");
    if (f == NULL)
    {
        printf("%s does not exist\n", file);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    int eof = ftell(f);
    rewind(f);

    char* res = calloc(eof + 1, sizeof(char));

    fread(res, eof, sizeof(char), f);

    fclose(f);

    return res;
}
