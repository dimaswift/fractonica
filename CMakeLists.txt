#
# project: fractonica
#
cmake_minimum_required(VERSION 3.21)
project(fractonica)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

set(USE_DAWN_SDK ON)

fips_setup()

set(HAS_COMPUTE_SHADERS ON)
add_definitions(-DSOKOL_NO_DEPRECATED)

if (FIPS_EMSCRIPTEN)
    set(sokol_backend SOKOL_WGPU)
    set(slang "wgsl")
    fips_libs(webgpu_dawn)
elseif (FIPS_ANDROID)
    set(sokol_backend SOKOL_GLES3)
    set(slang "glsl310es")
elseif (FIPS_WINDOWS)
    set(sokol_backend SOKOL_D3D11)
    set(slang "hlsl5")
endif ()

fips_import_dcimgui_dcimgui()
include_directories(libs)
add_subdirectory(src)
add_subdirectory(libs)