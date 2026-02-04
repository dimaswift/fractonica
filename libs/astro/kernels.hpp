#ifndef KERNELS_H
#define KERNELS_H
#include <string>

extern "C" {
  #include "../cspice/include/SpiceUsr.h"
}



int load_kernels_from_path(const char* kernel_path);
void unload_kernels(const char* kernel_path);

static int kernels_loaded = 0;


#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#endif

static std::string rootDir()
{
#if defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) return {};
    char resolved[PATH_MAX];
    if (!realpath(path, resolved)) return {};
    return std::string(dirname(resolved)); // .../MyApp.app/Contents/MacOS
#else
    return ".";
#endif
}

inline int load_kernels_from_path(std::string kernel_path) {

    furnsh_c((kernel_path + "/naif0012.tls").c_str());

    furnsh_c((kernel_path + "/de442s.bsp").c_str());

    furnsh_c((kernel_path + "/pck00011.tpc").c_str());

    kernels_loaded = 1;

    return 0;
}

inline void unload_kernels(std::string kernel_path) {
    if (!kernels_loaded) {
        return;
    }
    furnsh_c((kernel_path + "/naif0012.tls").c_str());

    furnsh_c((kernel_path + "/de442s.bsp").c_str());

    furnsh_c((kernel_path + "/pck00011.tpc").c_str());

    kernels_loaded = 0;
}

#endif // KERNELS_H