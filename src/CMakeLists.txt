
add_definitions(-D${sokol_backend})

fips_begin_app(main windowed)

fips_files(main.cpp DesktopApp.cpp UnixClock.h UnixClock.cpp ImGuiDisplay.cpp ImGuiInput.h)
sokol_shader(compute.glsl ${slang})
fips_deps(sokol imgui-docking)
fips_deps(core)

if(USE_CSPICE)
    fips_deps(cspice)
    cspice_copy_kernels()
    target_compile_definitions(main PRIVATE USE_CSPICE=1)
endif ()

fips_end_app()
