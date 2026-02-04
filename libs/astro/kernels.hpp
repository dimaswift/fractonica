#ifndef KERNELS_H
#define KERNELS_H

extern "C" {
  #include "../cspice/include/SpiceUsr.h"
}

int load_kernels_from_path(const char* kernel_path);
void unload_kernels(const char* kernel_path);

static int kernels_loaded = 0;

inline int load_kernels_from_path(const char* kernel_path) {

    static char path[64];
    sprintf(path,"%s/naif0012.tls", kernel_path);
    furnsh_c(path);

    sprintf(path,"%s/de442s.bsp", kernel_path);
    furnsh_c(path);

    sprintf(path,"%s/pck00011.tpc", kernel_path);
    furnsh_c(path);

    kernels_loaded = 1;

    return 0;
}

inline void unload_kernels(const char* kernel_path) {
    if (!kernels_loaded) {
        return;
    }
    static char path[64];
    snprintf(path, 64,"%s/naif0012.tls", kernel_path);
    unload_c(path);

    snprintf(path,64,"%s/de442s.bsp", kernel_path);
    unload_c(path);

    snprintf(path,64,"%s/pck00011.tpc", kernel_path);
    unload_c(path);

    kernels_loaded = 0;
}

#endif // KERNELS_H