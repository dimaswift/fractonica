cmake_minimum_required(VERSION 3.21)
project(fractonica-core)

add_library(core STATIC)

target_sources(core PRIVATE
        src/App.cpp
        src/Base8HeptClock.cpp
        src/Ephemeris.cpp
        src/LunarClockApp.cpp
        src/LunarGlyph.cpp
        src/LunarTime.cpp
        src/MatrixClock.cpp
        src/saros/lunar_impl.c
        src/saros/solar_impl.c
)

target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(core PUBLIC cxx_std_17)