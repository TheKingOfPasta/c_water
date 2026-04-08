#include "config_reloader.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

config* c = NULL;
static void* lib = NULL;

void reload_config(void)
{
#if defined(__NIXOS__)
    const char* cmd = "gcc -shared -fPIC -Iinclude config/config.c -o "
                      "config/config.so -D__NIXOS__";
#else
    const char* cmd = "gcc -shared -fPIC -Iinclude config/config.c -o config/config.so";
#endif

    if (system(cmd) != 0)
    {
        fprintf(stderr, "Failed to recompile config\n");
        return;
    }

    if (lib)
        dlclose(lib);

    lib = dlopen("./config/config.so", RTLD_NOW | RTLD_GLOBAL);
    if (!lib)
    {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return;
    }

    c = (config*)dlsym(lib, "c");
    if (!c)
    {
        fprintf(stderr, "dlsym: %s\n", dlerror());
        return;
    }
}
