add_definitions(-D${sokol_backend})

fips_begin_app(main windowed)
    fips_files(main.cpp)
    sokol_shader(compute.glsl ${slang})
    fips_deps(sokol imgui-docking)
fips_end_app()