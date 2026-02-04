
add_definitions(-D${sokol_backend})

fips_begin_app(main windowed)

fips_files(main.cpp)
sokol_shader(compute.glsl ${slang})
fips_deps(sokol imgui-docking)

if(USE_CSPICE)
    fips_deps(cspice)
    cspice_copy_kernels()
    target_compile_definitions(main PRIVATE USE_CSPICE=1)
endif ()

fips_end_app()
