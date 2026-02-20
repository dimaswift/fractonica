#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "DesktopApp.h"
#include "glyph.h"
#include "sokol_imgui.h"
#include "Mandelbrot.h"

struct AppState {
    sg_pass_action pass_action = {};
    Fractonica::Mandelbrot mandelbrot;
};

static AppState state;
static Fractonica::DesktopApp app;
static Fractonica::Glyph glyph;

static void draw_mandelbrot(const ImDrawList* dl, const ImDrawCmd* cmd) {
    (void)dl;

    const int cx = static_cast<int>(cmd->ClipRect.x);
    const int cy = static_cast<int>(cmd->ClipRect.y);
    const int cw = static_cast<int>(cmd->ClipRect.z - cmd->ClipRect.x);
    const int ch = static_cast<int>(cmd->ClipRect.w - cmd->ClipRect.y);

    sg_apply_scissor_rect(cx, cy, cw, ch, true);
    sg_apply_viewport(cx, cy, cw, ch, true);

    state.mandelbrot.draw();
}

void init() {
    app.setup();
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    // Default pass action
    state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    state.pass_action.colors[0].clear_value = { 0.1f, 0.1f, 0.1f, 1.0f };

    // Setup ImGui
    simgui_desc_t simgui_desc{};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    state.mandelbrot.setup(512, 512);
}

void frame() {
    state.mandelbrot.compute();


    // ========================================
    // IMGUI UI
    // ========================================
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = sapp_frame_duration();
    const float dpi_scale = sapp_dpi_scale();
    simgui_frame_desc_t frame_desc{};
    frame_desc.width = width;
    frame_desc.height = height;
    frame_desc.delta_time = delta_time;
    frame_desc.dpi_scale = dpi_scale;
    simgui_new_frame(&frame_desc);

    glyph.render(delta_time);

    //app.run();
    // ImGui::SetNextWindowPos(ImVec2(512, 80), ImGuiCond_Once);
    // ImGui::SetNextWindowSize(ImVec2(state.mandelbrot.getWidth() + 10, state.mandelbrot.getHeight() + 140), ImGuiCond_Once);
    //
    // if (ImGui::Begin("Mandelbrot Fractal")) {
    //
    //     state.mandelbrot.drawGui();
    //
    //     ImGui::Separator();
    //
    //     if (ImGui::BeginChild("Fractal View", ImVec2(state.mandelbrot.getWidth(), state.mandelbrot.getHeight()), true, ImGuiWindowFlags_None)) {
    //         ImDrawList* dl = ImGui::GetWindowDrawList();
    //         dl->AddCallback(draw_mandelbrot, nullptr);
    //     }
    //
    //     ImGui::EndChild();
    // }
    // ImGui::End();


    sg_pass render_pass{};
    render_pass.action = state.pass_action;
    render_pass.swapchain = sglue_swapchain();
    render_pass.label = "render-pass";
    sg_begin_pass(&render_pass);
    simgui_render();

    sg_end_pass();
    sg_commit();
}

void input(const sapp_event* ev) {
    simgui_handle_event(ev);
}

void cleanup() {
    app.shutdown();
    state.mandelbrot.shutdown();
    simgui_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = 1200,
        .height = 1200,
        .window_title = "Fractonica",
        .logger.func = slog_func,
    };
}