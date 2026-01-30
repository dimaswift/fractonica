cmake_minimum_required(VERSION 3.15)
project(fractonica LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This CMakeLists.txt is WASM-only. Configure with emcmake/em++.")
endif()

# -----------------------
# Paths
# -----------------------
set(CORE_DIR    "${CMAKE_SOURCE_DIR}/lib/core")
set(DESKTOP_DIR "${CMAKE_SOURCE_DIR}/lib/desktop")
set(IMGUI_DIR   "${CMAKE_SOURCE_DIR}/lib/imgui")

# -----------------------
# core library
# -----------------------
file(GLOB_RECURSE CORE_SOURCES
        "${CORE_DIR}/src/*.c"
        "${CORE_DIR}/src/*.cc"
        "${CORE_DIR}/src/*.cpp"
        "${CORE_DIR}/src/*.cxx"
)

add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC
        "${CORE_DIR}/include"
)

# -----------------------
# imgui library (with emscripten-friendly backend selection)
# Prefer imgui_impl_glfw + imgui_impl_opengl3 for WebGL via Emscripten
# -----------------------
add_library(imgui STATIC
        "${IMGUI_DIR}/src/imgui.cpp"
        "${IMGUI_DIR}/src/imgui_draw.cpp"
        "${IMGUI_DIR}/src/imgui_demo.cpp"
        "${IMGUI_DIR}/src/imgui_widgets.cpp"
        "${IMGUI_DIR}/src/imgui_tables.cpp"
        "${IMGUI_DIR}/src/backends/imgui_impl_glfw.cpp"
        "${IMGUI_DIR}/src/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
        "${IMGUI_DIR}/include"
        "${IMGUI_DIR}/include/backends"
)

# If you use OpenGL ES/WebGL you typically want this define for the OpenGL3 backend:
# (pick the version string you use in your code; common: "#version 300 es")
target_compile_definitions(imgui PUBLIC
        IMGUI_IMPL_OPENGL_ES3=1
)

# imgui_impl_glfw needs GLFW headers. With Emscripten this is provided by the SDK,
# so you usually don't add include dirs; just link flags later (-sUSE_GLFW=3).

# -----------------------
# desktop library
# -----------------------
file(GLOB_RECURSE DESKTOP_SOURCES
        "${DESKTOP_DIR}/src/*.c"
        "${DESKTOP_DIR}/src/*.cc"
        "${DESKTOP_DIR}/src/*.cpp"
        "${DESKTOP_DIR}/src/*.cxx"
)

add_library(desktop STATIC ${DESKTOP_SOURCES})
target_include_directories(desktop PUBLIC
        "${DESKTOP_DIR}/include"
)

# Link core + imgui into desktop (PUBLIC so dependents inherit them)
target_link_libraries(desktop PUBLIC core imgui)

# -----------------------
# executable
# -----------------------
add_executable(fractonica
        "src/main.cpp"
)

target_link_libraries(fractonica PRIVATE desktop)

set_target_properties(fractonica PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
)

# -----------------------
# Emscripten link options
# -----------------------
target_link_options(fractonica PRIVATE
        "-sUSE_GLFW=3"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-sWASM=1"
        "--shell-file" "${CMAKE_SOURCE_DIR}/web/index.html"
        "--preload-file" "${CMAKE_SOURCE_DIR}/assets@assets"
)