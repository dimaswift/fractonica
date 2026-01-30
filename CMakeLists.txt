cmake_minimum_required(VERSION 3.15)
project(fractonica LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CORE_DIR    "${CMAKE_SOURCE_DIR}/lib/core")
set(DESKTOP_DIR "${CMAKE_SOURCE_DIR}/lib/desktop")
set(IMGUI_DIR   "${CMAKE_SOURCE_DIR}/lib/imgui")

# iOS detect
set(IS_IOS FALSE)
if(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IS_IOS TRUE)
endif()

# -----------------------
# core
# -----------------------
file(GLOB_RECURSE CORE_SOURCES
        "${CORE_DIR}/src/*.c" "${CORE_DIR}/src/*.cc" "${CORE_DIR}/src/*.cpp" "${CORE_DIR}/src/*.cxx"
)
add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC "${CORE_DIR}/include")

# -----------------------
# GLFW (native desktop only)
# -----------------------
if(NOT EMSCRIPTEN AND NOT IS_IOS)
    include(FetchContent)
    FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# -----------------------
# imgui (Metal on iOS, GLFW+GL elsewhere incl. wasm)
# -----------------------
set(IMGUI_SOURCES
        "${IMGUI_DIR}/src/imgui.cpp"
        "${IMGUI_DIR}/src/imgui_draw.cpp"
        "${IMGUI_DIR}/src/imgui_demo.cpp"
        "${IMGUI_DIR}/src/imgui_widgets.cpp"
        "${IMGUI_DIR}/src/imgui_tables.cpp"
)

if(IS_IOS)
    list(APPEND IMGUI_SOURCES "${IMGUI_DIR}/src/backends/imgui_impl_metal.mm")
else()
    list(APPEND IMGUI_SOURCES
            "${IMGUI_DIR}/src/backends/imgui_impl_glfw.cpp"
            "${IMGUI_DIR}/src/backends/imgui_impl_opengl3.cpp"
    )
endif()

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        "${IMGUI_DIR}/include"
        "${IMGUI_DIR}/include/backends"
)

if(EMSCRIPTEN)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_ES3=1)
elseif(NOT IS_IOS)
    target_link_libraries(imgui PUBLIC glfw)
endif()

# -----------------------
# desktop lib (build for native + wasm, not iOS)
# -----------------------
if(NOT IS_IOS)
    file(GLOB_RECURSE DESKTOP_SOURCES
            "${DESKTOP_DIR}/src/*.c" "${DESKTOP_DIR}/src/*.cc" "${DESKTOP_DIR}/src/*.cpp" "${DESKTOP_DIR}/src/*.cxx"
    )
    add_library(desktop STATIC ${DESKTOP_SOURCES})
    target_include_directories(desktop PUBLIC "${DESKTOP_DIR}/include")
    target_link_libraries(desktop PUBLIC core imgui)
endif()

# -----------------------
# Targets
# -----------------------
if(IS_IOS)
    add_executable(fractonica_ios
            ios/main.mm
            ios/AppDelegate.mm
            ios/ViewController.mm
    )
    target_link_libraries(fractonica_ios PRIVATE core imgui
            "-framework UIKit"
            "-framework Foundation"
            "-framework QuartzCore"
            "-framework Metal"
            "-framework MetalKit"
    )

    set(APP_BUNDLE_ID "com.dimaswift.fractonica")
    set(APP_VERSION   "0.0.3")
    set(APP_BUILD     "1")

    configure_file(
            "${CMAKE_SOURCE_DIR}/ios/Info.plist.in"
            "${CMAKE_BINARY_DIR}/ios/Info.plist"
            @ONLY
    )

    set_target_properties(fractonica_ios PROPERTIES
            MACOSX_BUNDLE YES
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${APP_BUNDLE_ID}"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/ios/Info.plist"
    )

elseif(EMSCRIPTEN)
    add_executable(fractonica "src/main.cpp")
    target_link_libraries(fractonica PRIVATE desktop)

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
    add_executable(fractonica "src/main.cpp")
    target_link_libraries(fractonica PRIVATE desktop)

    # (optional) native-only extra links, like you had before:
    if(WIN32)
        target_link_libraries(fractonica PRIVATE opengl32 gdi32 user32 shell32)
    elseif(APPLE)
        find_package(OpenGL REQUIRED)
        target_link_libraries(fractonica PRIVATE OpenGL::GL "-framework Cocoa" "-framework IOKit")
    else()
        find_package(OpenGL REQUIRED)
        target_link_libraries(fractonica PRIVATE OpenGL::GL ${CMAKE_DL_LIBS})
    endif()
endif()