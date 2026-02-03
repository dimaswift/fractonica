
//------------------------------------------------------------------------------
//  imgui-dock-sapp.cpp
//
//  Converted to C++20
//------------------------------------------------------------------------------
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"
#include "vecmath/vecmath.h"
#include "compute.glsl.h"
#include <array>
#include <cmath>

// Use constexpr instead of #define for type safety
constexpr int MAX_PARTICLES = 512 * 1024;
constexpr int NUM_PARTICLES_EMITTED_PER_FRAME = 10;

using namespace vecmath;

struct ComputeState {
    sg_view sbuf_view;
    sg_pipeline pip;
};

struct DisplayState {
    sg_buffer vbuf;
    sg_buffer ibuf;
    sg_pipeline pip;
    sg_pass_action pass_action;
};

// Encapsulate global state in a named struct
struct AppState {
    int num_particles = 0;
    float ry = 0.0f;
    sg_buffer buf;
    ComputeState compute;
    DisplayState display;
    bool show_test_window = true;
    bool show_another_window = false;
};

static AppState state{};

static vs_params_t compute_vsparams(float frame_time) {
    const mat44_t proj = mat44_perspective_fov_rh(vecmath_radians(60.0f), sapp_widthf()/sapp_heightf(), 0.01f, 50.0f);
    const mat44_t view = mat44_look_at_rh(vec3(0.0f, 1.5f, 8.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    const mat44_t view_proj = mat44_mul_mat44(view, proj);
    
    state.ry += 60.0f * frame_time;
    
    return vs_params_t {
        .mvp = mat44_mul_mat44(mat44_rotation_y(vecmath_radians(state.ry)), view_proj),
    };
}

static void init() {
    // setup sokol-gfx, sokol-time and sokol-imgui
    sg_desc desc{};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    // Particle buffer
    sg_buffer_desc buf_desc{};
    buf_desc.size = MAX_PARTICLES * sizeof(particle_t);
    buf_desc.usage.vertex_buffer = true;
    buf_desc.usage.storage_buffer = true;
    buf_desc.label = "particle-buffer";

    state.buf = sg_make_buffer(&buf_desc);

    // Compute resource view
    sg_view_desc particle_view_desc{};
    particle_view_desc.storage_buffer.buffer = state.buf;
    particle_view_desc.label = "particle-buffer-view";

    state.compute.sbuf_view = sg_make_view(&particle_view_desc);

    // Update pipeline
    sg_pipeline_desc pipeline_desc{};
    pipeline_desc.compute = true;
    pipeline_desc.shader = sg_make_shader(update_shader_desc(sg_query_backend()));
    pipeline_desc.label = "update-pipeline";

    state.compute.pip = sg_make_pipeline(&pipeline_desc);

    const float r = 0.05f;
    // Use std::array for C++ safety (optional, but good practice)
    const std::array<float, 42> vertices = {
        // positions            colors
        0.0f,   -r, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f,
           r, 0.0f, r,          0.0f, 1.0f, 0.0f, 1.0f,
           r, 0.0f, -r,         0.0f, 0.0f, 1.0f, 1.0f,
          -r, 0.0f, -r,         1.0f, 1.0f, 0.0f, 1.0f,
          -r, 0.0f, r,          0.0f, 1.0f, 1.0f, 1.0f,
        0.0f,    r, 0.0f,       1.0f, 0.0f, 1.0f, 1.0f
    };
    
    const std::array<uint16_t, 24> indices = {
        0, 1, 2,    0, 2, 3,    0, 3, 4,    0, 4, 1,
        5, 1, 2,    5, 2, 3,    5, 3, 4,    5, 4, 1
    };

    // Geometry Vertex Buffer
    sg_buffer_desc gbuffer_desc{};
    gbuffer_desc.data = SG_RANGE(vertices);
    gbuffer_desc.label = "geometry-vbuf";
    state.display.vbuf = sg_make_buffer(&gbuffer_desc);

    // Index Buffer
    sg_buffer_desc ibuffer_desc{};
    ibuffer_desc.usage.index_buffer = true;
    ibuffer_desc.data = SG_RANGE(indices);
    ibuffer_desc.label = "geometry-ibuf";
    state.display.ibuf = sg_make_buffer(&ibuffer_desc);

    // Render Pipeline
    sg_pipeline_desc render_pip_desc{};
    render_pip_desc.shader = sg_make_shader(display_shader_desc(sg_query_backend()));
    
    // Layout setup
    render_pip_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    render_pip_desc.layout.buffers[1].stride = sizeof(particle_t);
    
    render_pip_desc.layout.attrs[ATTR_display_pos].format = SG_VERTEXFORMAT_FLOAT3;
    render_pip_desc.layout.attrs[ATTR_display_color0].format = SG_VERTEXFORMAT_FLOAT4;
    render_pip_desc.layout.attrs[ATTR_display_inst_pos].format = SG_VERTEXFORMAT_FLOAT4;
    render_pip_desc.layout.attrs[ATTR_display_inst_pos].buffer_index = 1;

    render_pip_desc.index_type = SG_INDEXTYPE_UINT16;
    render_pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    render_pip_desc.depth.write_enabled = true;
    render_pip_desc.cull_mode = SG_CULLMODE_BACK;
    render_pip_desc.label = "render-pipeline";

    state.display.pip = sg_make_pipeline(&render_pip_desc);

    // One-time init compute pass
    sg_pipeline_desc init_pd{};
    init_pd.compute = true;
    init_pd.shader = sg_make_shader(init_shader_desc(sg_query_backend()));

    sg_pipeline pip = sg_make_pipeline(&init_pd);
    
    sg_pass pass{};
    pass.compute = true;
    sg_begin_pass(&pass);
    sg_apply_pipeline(pip);
    
    sg_bindings binds{};
    binds.views[VIEW_cs_ssbo] = state.compute.sbuf_view;
    sg_apply_bindings(&binds);
    
    sg_dispatch(MAX_PARTICLES / 64, 1, 1);
    sg_end_pass();
    sg_destroy_pipeline(pip);

    // ImGui Setup
    simgui_desc_t simgui_desc{};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

static void frame() {
    state.num_particles += NUM_PARTICLES_EMITTED_PER_FRAME;
    if (state.num_particles > MAX_PARTICLES) {
        state.num_particles = MAX_PARTICLES;
    }
    
    const float dt = static_cast<float>(sapp_frame_duration());

    // --- Compute Pass ---
    const cs_params_t cs_params = {
        .dt = dt,
        .num_particles = state.num_particles,
    };
    
    sg_pass compute_pass{};
    compute_pass.compute = true;
    compute_pass.label = "compute-pass";
    
    sg_begin_pass(&compute_pass);
    sg_apply_pipeline(state.compute.pip);
    
    sg_bindings compute_binds{};
    compute_binds.views[VIEW_cs_ssbo] = state.compute.sbuf_view;
    sg_apply_bindings(&compute_binds);

    sg_apply_uniforms(UB_cs_params, SG_RANGE(cs_params));
    sg_dispatch((state.num_particles + 63) / 64, 1, 1);
    sg_end_pass();

    // --- Render Pass ---
    const vs_params_t vs_params = compute_vsparams(dt);
    
    sg_pass render_pass{};
    render_pass.action = state.display.pass_action;
    render_pass.swapchain = sglue_swapchain();
    render_pass.label = "render-pass";
    
    sg_begin_pass(&render_pass);
    sg_apply_pipeline(state.display.pip);
    
    sg_bindings render_binds{};
    render_binds.vertex_buffers[0] = state.display.vbuf;
    render_binds.vertex_buffers[1] = state.buf;
    render_binds.index_buffer = state.display.ibuf;
    sg_apply_bindings(&render_binds);
    
    sg_apply_uniforms(UB_vs_params, SG_RANGE(vs_params));
    sg_draw(0, 24, state.num_particles);

    // --- ImGui ---
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = sapp_frame_duration();
    const float dpi_scale = sapp_dpi_scale();
    
    // Explicit constructor for C++ clarity
    simgui_frame_desc_t frame_desc{};
    frame_desc.width = width;
    frame_desc.height = height;
    frame_desc.delta_time = delta_time;
    frame_desc.dpi_scale = dpi_scale;
    simgui_new_frame(&frame_desc);

    static float f = 0.0f;
    ImGui::Text("Drag windows over one another!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", &state.display.pass_action.colors[0].clear_value.r);
    
    if (ImGui::Button("Test Window")) state.show_test_window = !state.show_test_window;
    if (ImGui::Button("Another Window")) state.show_another_window = !state.show_another_window;
    
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (state.show_another_window) {
        ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Another Window", &state.show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }

    if (state.show_test_window) {
        ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup() {
    simgui_shutdown();
    sg_shutdown();
}

static void input(const sapp_event* event) {
    simgui_handle_event(event);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    // Silence unused parameter warnings
    (void)argc; 
    (void)argv;
    
    sapp_desc desc{};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.width = 1024;
    desc.height = 768;
    desc.window_title = "fractonica";
    desc.ios.keyboard_resizes_canvas = false;
    desc.icon.sokol_default = true;
    desc.enable_clipboard = true;
    desc.logger.func = slog_func;
    return desc;
}
