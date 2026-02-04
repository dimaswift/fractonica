cmake_minimum_required(VERSION 3.21)
project(fractonica)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(core)

set(FIPS_IMPORT 1)

if (FIPS_IMPORT)

    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")
    include("${FIPS_ROOT_DIR}/../cspice/fips.cmake")

    if (USE_CSPICE)
        cspice_fetch_kernels()
    endif()

    if (FIPS_EMSCRIPTEN)
        set(USE_DAWN_SDK ON)
    endif()

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
    elseif (FIPS_OSX)
        set(sokol_backend SOKOL_METAL)
        if (FIPS_IOS)
            set(slang "metal_ios:metal_sim")
        else()
            set(slang "metal_macos")
        endif()
    endif ()

    fips_import_dcimgui_dcimgui()
    include_directories(libs)
    add_subdirectory(src)
    add_subdirectory(libs)


else ()

    get_filename_component(ROOT_DIR "../" ABSOLUTE)

    add_library(imgui
             ${ROOT_DIR}/dcimgui/src/cimgui.cpp
             ${ROOT_DIR}/dcimgui/src/cimgui_internal.cpp
             ${ROOT_DIR}/dcimgui/src/imgui_demo.cpp
             ${ROOT_DIR}/dcimgui/src/imgui_draw.cpp
             ${ROOT_DIR}/dcimgui/src/imgui_tables.cpp
             ${ROOT_DIR}/dcimgui/src/imgui_widgets.cpp
             ${ROOT_DIR}/dcimgui/src/imgui.cpp)

    add_library(imgui-docking
             ${ROOT_DIR}/dcimgui/src-docking/cimgui.cpp
             ${ROOT_DIR}/dcimgui/src-docking/cimgui_internal.cpp
             ${ROOT_DIR}/dcimgui/src-docking/imgui_demo.cpp
             ${ROOT_DIR}/dcimgui/src-docking/imgui_draw.cpp
             ${ROOT_DIR}/dcimgui/src-docking/imgui_tables.cpp
             ${ROOT_DIR}/dcimgui/src-docking/imgui_widgets.cpp
             ${ROOT_DIR}/dcimgui/src-docking/imgui.cpp)

    include_directories(${ROOT_DIR}/sokol)
    include_directories(${ROOT_DIR}/sokol/util)
    include_directories(${ROOT_DIR}/dcimgui/src-docking)
    include_directories(libs)

    include_directories(${ROOT_DIR}/fips-build/fractonica/win/src)
    include_directories(core/include)

    target_include_directories(imgui PUBLIC ${ROOT_DIR}/dcimgui/src)
    target_include_directories(imgui-docking PUBLIC ${ROOT_DIR}/dcimgui/src-docking)

if ((${CMAKE_CXX_COMPILER_ID} MATCHES "Clang") OR (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU"))
    target_compile_options(imgui PUBLIC -Wno-return-type-c-linkage -Wno-unused-function)

    target_compile_options(imgui-docking PUBLIC -Wno-return-type-c-linkage -Wno-unused-function)
elseif (MSVC)
    target_compile_options(imgui PUBLIC /wd4190)
    target_compile_options(imgui-docking PUBLIC /wd4190)
endif ()


    add_executable(main src/main.cpp src/ImGuiDisplay.cpp)

endif ()
