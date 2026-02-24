#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_time.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "DesktopApp.h"
#include "sokol_imgui.h"
//#include "Mandelbrot.h"
#include "OctalGlyph.h"
#include "saros.h"

struct SarosState {
    uint8_t number;
    bool visible;
    Fractonica::OctalGlyphSettings settings;
    bool configure;
    constexpr SarosState(uint8_t number, Fractonica::OctalGlyphSettings s) : number(number), visible(true), settings(s) {}
    bool operator==(const SarosState& other) const {
        return number == other.number;
    }
    bool operator!=(const SarosState& other) const {
        return !(*this == other);
    }
};

struct AppState {
    sg_pass_action pass_action = {};
   // Fractonica::Mandelbrot mandelbrot;
};

static Fractonica::OctalGlyphSettings settings;
static AppState state;
static Fractonica::DesktopApp app;
static Fractonica::ImGuiDisplay display(512, 512, 1, "Test");
static std::vector sarosNumbers = {
    SarosState(141, settings),
    SarosState(128, settings),
    SarosState(118, settings)};

static void draw_mandelbrot(const ImDrawList* dl, const ImDrawCmd* cmd) {
    (void)dl;

    const int cx = static_cast<int>(cmd->ClipRect.x);
    const int cy = static_cast<int>(cmd->ClipRect.y);
    const int cw = static_cast<int>(cmd->ClipRect.z - cmd->ClipRect.x);
    const int ch = static_cast<int>(cmd->ClipRect.w - cmd->ClipRect.y);

    sg_apply_scissor_rect(cx, cy, cw, ch, true);
    sg_apply_viewport(cx, cy, cw, ch, true);

   // state.mandelbrot.draw();
}

void init() {

    app.setup();
    stm_setup();
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

   // state.mandelbrot.setup(512, 512);
}

void frame() {
   // state.mandelbrot.compute();


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

    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Saros")) {

            if (ImGui::BeginMenu("Add")) {

                static int saros = 141;
                ImGui::InputInt("Saros", &saros);
                if (ImGui::Button("Add")) {
                    if (std::find(sarosNumbers.begin(), sarosNumbers.end(), SarosState(saros, settings)) == sarosNumbers.end())
                        sarosNumbers.emplace_back(saros, settings);
                }

                ImGui::EndMenu();
            }


            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    int x = 32;

    for (size_t i = 0; i < sarosNumbers.size(); ++i) {
        SarosState saros = sarosNumbers[i];

        if (saros.configure) {
            ImGui::Begin("Settings", &saros.configure);
            int type = (saros.settings.type);
            ImGui::RadioButton("Pixel", &type, 0); ImGui::SameLine();
            ImGui::RadioButton("Line", &type, 1); ImGui::SameLine();
            ImGui::RadioButton("Path", &type, 2);
            ImGui::Checkbox("Labels", &saros.settings.showLabels);ImGui::SameLine();
            ImGui::Checkbox("Horizontal", &saros.settings.horizontal);ImGui::SameLine();
            ImGui::Checkbox("Border", &saros.settings.showBorder);
            ImGui::SliderFloat("Size", &saros.settings.size, 2, 32);
            ImGui::SliderFloat("Thickness", &saros.settings.thickness, 0.5, 8);
            constexpr int min = 1;
            constexpr int max = 8;
            ImGui::SliderScalar("Limit", ImGuiDataType_U8, &saros.settings.symbolLimit, &min, &max);
            saros.settings.type = static_cast<Fractonica::OctalGlyphType>(type);
            settings = saros.settings;
            ImGui::End();
        }

        char name[8];
        snprintf(name, sizeof(name), "%03d", saros.number);
        ImGui::SetNextWindowPos(ImVec2(x, 32), ImGuiCond_Once);
        x+=164;
        ImGui::Begin(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize);

       const auto pos = ImGui::GetCursorScreenPos();
       const auto v = calculate_solar_octal_phase_ms(seconds, saros.number, 12);
       Fractonica::OctalGlyph::Draw(v, &display, Vector2(pos.x, pos.y ), saros.settings);
        if (ImGui::BeginPopupContextItem(name))
        {
            if (ImGui::MenuItem("Settings")) {
                saros.configure = true;

            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                sarosNumbers.erase(sarosNumbers.begin() + i);
                --i;
                ImGui::EndPopup();
                ImGui::End();
                break;
            }
            ImGui::EndPopup();
        }
       ImGui::End();
       sarosNumbers[i] = saros;
   }

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
   // state.mandelbrot.shutdown();
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