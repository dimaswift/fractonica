#if defined(PLATFORM_ARDUINO)

  #include <ArduinoBoot.h>

#elif defined(PLATFORM_MEGA2560)

  #include <Mega2560Boot.h>

#else  // Desktop / WASM

  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define GLFW_INCLUDE_ES3
    #include <GLES3/gl3.h>
  #elif defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
  #else
    #include <GL/gl.h>
  #endif

  #include <GLFW/glfw3.h>

  #include <imgui.h>
  #include "backends/imgui_impl_glfw.h"
  #include "backends/imgui_impl_opengl3.h"

  #include <cstdio>
  #include <memory>

  #include "DesktopApp.h"   // from lib/desktop/include in your CMake

namespace {
  GLFWwindow* g_window = nullptr;
  std::unique_ptr<Fractonica::DesktopApp> g_app;
  ImVec4 g_clear_color{0.0f, 0.0f, 0.0f, 1.0f};

  const char* glsl_version() {
  #ifdef __EMSCRIPTEN__
    return "#version 300 es";
  #else
    return "#version 150";
  #endif
  }

  bool init_glfw_and_window() {
    if (!glfwInit()) {
      std::fprintf(stderr, "Failed to initialize GLFW\n");
      return false;
    }

  #ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif

  #ifdef __EMSCRIPTEN__
    g_window = glfwCreateWindow(1, 1, "fractonica", nullptr, nullptr);
  #else
    g_window = glfwCreateWindow(1280, 720, "fractonica", nullptr, nullptr);
  #endif

    if (!g_window) {
      std::fprintf(stderr, "Failed to create GLFW window\n");
      return false;
    }

    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1);
    return true;
  }

  bool init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    if (!ImGui_ImplGlfw_InitForOpenGL(g_window, /*install_callbacks=*/true)) {
      std::fprintf(stderr, "ImGui_ImplGlfw_InitForOpenGL failed\n");
      return false;
    }

  #ifdef __EMSCRIPTEN__
    // Ensure your HTML has: <canvas id="canvas"> ... </canvas>
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(g_window, "#canvas");
  #endif

    if (!ImGui_ImplOpenGL3_Init(glsl_version())) {
      std::fprintf(stderr, "ImGui_ImplOpenGL3_Init failed\n");
      return false;
    }

    // Fonts: must be reachable at runtime (preloaded for WASM)
    io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("assets/fonts/primary.ttf", 16.0f);

    return true;
  }

  void shutdown_everything() {
    if (g_app) {
      g_app->cleanup();
      g_app.reset();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (g_window) {
      glfwDestroyWindow(g_window);
      g_window = nullptr;
    }
    glfwTerminate();
  }

  void frame() {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    g_app->run();

    ImGui::Render();

    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);

    glViewport(0, 0, display_w, display_h);
    glClearColor(g_clear_color.x, g_clear_color.y, g_clear_color.z, g_clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  #ifndef __EMSCRIPTEN__
    glfwSwapBuffers(g_window);
  #endif
  }

#ifdef __EMSCRIPTEN__
  void emscripten_main_loop_trampoline() {
    // Optionally stop when the window is requested to close (if you wire that up)
    frame();
  }
#endif
} // namespace

int main(int /*argc*/, char** /*argv*/) {
  if (!init_glfw_and_window()) {
    shutdown_everything();
    return 1;
  }

  if (!init_imgui()) {
    shutdown_everything();
    return 1;
  }

  g_app = std::make_unique<Fractonica::DesktopApp>();
  g_app->setup();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(emscripten_main_loop_trampoline, 0, 1);
  // NOTE: code below is not reached unless you use emscripten_set_main_loop_arg + cancel
  return 0;
#else
  while (!glfwWindowShouldClose(g_window)) {
    frame();
  }
  shutdown_everything();
  return 0;
#endif
}

#endif