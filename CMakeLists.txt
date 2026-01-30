cmake_minimum_required(VERSION 3.15)
project(fractonica LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
# GLFW setup (only for native builds)
# -----------------------
if(NOT EMSCRIPTEN)
    include(FetchContent)
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )

    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(glfw)
endif()

# -----------------------
# imgui library
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

# Platform-specific imgui configuration
if(EMSCRIPTEN)
    target_compile_definitions(imgui PUBLIC
            IMGUI_IMPL_OPENGL_ES3=1
    )
else()
    # Native builds: link GLFW to imgui so it can find headers
    target_link_libraries(imgui PUBLIC glfw)
endif()

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

target_link_libraries(desktop PUBLIC core imgui)

# -----------------------
# executable
# -----------------------
add_executable(fractonica
        "src/main.cpp"
)

target_link_libraries(fractonica PRIVATE desktop)

# -----------------------
# Platform-specific configuration
# -----------------------
if(EMSCRIPTEN)
    # WASM build
    set_target_properties(fractonica PROPERTIES
            OUTPUT_NAME "index"
            SUFFIX ".html"
    )

    target_link_options(fractonica PRIVATE
            "-sUSE_GLFW=3"
            "-sUSE_WEBGL2=1"
            "-sFULL_ES3=1"
            "-sWASM=1"
            "--shell-file" "${CMAKE_SOURCE_DIR}/web/index.html"
            "--preload-file" "${CMAKE_SOURCE_DIR}/assets@assets"
    )

    set(WEB_DIR "${CMAKE_SOURCE_DIR}/web/www")

    add_custom_command(TARGET fractonica POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${WEB_DIR}"
            "$<TARGET_FILE_DIR:fractonica>"
            COMMENT "Copying web/ assets to build output"
    )
else()
    # Native Windows/Desktop build
    target_link_libraries(fractonica PRIVATE glfw)

    # Windows-specific libraries
    if(WIN32)
        target_link_libraries(fractonica PRIVATE
                opengl32
                gdi32
                user32
                shell32
        )
    elseif(APPLE)
        find_package(OpenGL REQUIRED)
        target_link_libraries(fractonica PRIVATE
                OpenGL::GL
                "-framework Cocoa"
                "-framework IOKit"
        )
    else() # Linux
        find_package(OpenGL REQUIRED)
        target_link_libraries(fractonica PRIVATE
                OpenGL::GL
                ${CMAKE_DL_LIBS}
        )
    endif()
endif()