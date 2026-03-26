#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_time.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#define SOKOL_GL_IMPL


#include "DesktopApp.h"
#include "sokol_imgui.h"
//#include "Mandelbrot.h"
#include "Audio.h"
#include "GuiUtils.h"
#include "OctalGlyph.h"
#include "saros.h"
#include "SolidExplorer.h"
#include "Synth.h"
#include "Utils.h"
#include "../core/include/ToneGenerator.h"


struct SarosState {
    uint8_t number = 0;
    bool visible = true;
    uint64_t lastValue = 0;
    double timer = 0;
    Fractonica::OctalGlyphSettings settings;
    bool configure = false;
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
    bool showExplorer = false;
    bool showSolidsExplorer = true;
    bool enableSound = false;
    bool showWaveformEditor = false;
    float frequency = 11;
    float offset = 76;
    float amp = 66;
   // Fractonica::Mandelbrot mandelbrot;
};

static Fractonica::ToneGenerator tone_generator(64, 44100);
static Fractonica::Audio audio;
static Fractonica::Synth synth;
static Fractonica::OctalGlyphSettings settings;
static AppState state;
static Fractonica::DesktopApp app;
static Fractonica::ImGuiDisplay display(512, 512, 1, "Test");
static std::vector<SarosState> sarosNumbers = {};
static Fractonica::SolidExplorer solid_explorer;

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


void HandleAudio(float* buffer, int num_frames, int num_channels, void* user_data) {
    for (int i = 0; i < num_frames; ++i) {
        int16_t sample = synth.Sample();
        for (int c = 0; c < num_channels; ++c) {
            *buffer++ = (float) sample / 32767.0f;
        }
    }
}



void init() {

    audio.Initialize(HandleAudio);
    app.setup();
    stm_setup();
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);
    sgl_setup({
          .logger.func = slog_func,
    });

    // Default pass action
    state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    state.pass_action.colors[0].clear_value = { 0.1f, 0.1f, 0.1f, 1.0f };

    // Setup ImGui
    simgui_desc_t simgui_desc{};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    synth.SetSampleRate(audio.GetSampleRate());

    for (int i = 0; i < ALIVE_SAROS_COUNT; ++i) {
        sarosNumbers.emplace_back(SarosOrderedByBirth[i], settings);
    }

    tone_generator.Randomize(state.frequency, state.amp);
   // state.mandelbrot.setup(512, 512);

    solid_explorer.init();
}


void ModulateTone(uint32_t sample_counter, uint32_t base_phase_inc, int32_t base_vol, uint32_t duration_samples, int32_t & out_phase_inc, int32_t & out_vol) {
    tone_generator.ModulateFast(sample_counter, base_phase_inc, base_vol, duration_samples, out_phase_inc, out_vol);
}

void draw_saros_glyphs() {

    const auto now = std::chrono::system_clock::now();
    const double delta_time = sapp_frame_duration();
    const auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    int x = 32;
    int y = 32;
    for (int i = sarosNumbers.size() - 1; i >= 0; --i) {
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
        ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);
        x+=96;
        if (i % 10 == 0) {
            x = 32;
            y += 144;
        }
        ImGui::Begin(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar);

       const auto pos = ImGui::GetCursorScreenPos();
       // ImGui::Text("%lld", seconds);
        const auto v = calculate_solar_octal_phase_ms(seconds, saros.number, 2);
        if (saros.timer > 0) {
            saros.timer -= delta_time;
            saros.settings.color = Fractonica::Utils::ColorHSV(saros.timer * 0x8000, 255,255);
        }
        else {
            saros.settings.color = Fractonica::Utils::ColorHSV(0x8000, 255,255);
        }

        Fractonica::OctalGlyph::Draw(v, &display, Vector2(pos.x, pos.y ), saros.settings);

        static constexpr float notes[8] = {
            261.63,
            277.18,
            293.66,
            311.13,
            329.63,
            349.23,
            369.99,
            392.00
        };

        auto sonifiedValue = v >> 0;

        if (saros.lastValue != sonifiedValue) {

            if (saros.lastValue != 0) {
                uint8_t zeroes = 0;
                uint8_t p = 0;
                while (true) {
                    auto d = (sonifiedValue >> (3 * p)) & 7;
                    p++;
                    if (d != 0) break;
                    zeroes++;
                }
                auto d0 = (sonifiedValue >> (3 * 0)) & 7;
                saros.timer = 2.25 + (pow(2, zeroes));
                if (state.enableSound) {

                    synth.PlayVoice(i, notes[d0], 0.5f, saros.timer, Fractonica::Synth::OscSine, ModulateTone);
                }
            }
            saros.lastValue = sonifiedValue;

        }
        if (ImGui::BeginPopupContextItem(name))
        {
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
}

void frame() {
   // state.mandelbrot.compute();

    // ========================================
    // IMGUI UI
    // ========================================
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = sapp_frame_duration();
    const uint64_t frameCount = sapp_frame_count();
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

            if (ImGui::MenuItem("Explore")) {
                state.showExplorer = true;

            }

            if (ImGui::MenuItem("Edit Waveforms")) {
                state.showWaveformEditor = true;

            }

            if (ImGui::BeginMenu("Settings")) {
                ImGui::Checkbox("Enable Sound", &state.enableSound);
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (state.showSolidsExplorer) {
        ImGui::SetNextWindowSize(ImVec2(512, 1024), ImGuiCond_Once);
        ImGui::Begin("Solids", &state.showSolidsExplorer);


        ImGui::End();
    }


    if (state.showExplorer) {
        ImGui::SetNextWindowSize(ImVec2(512, 1024), ImGuiCond_Once);
        ImGui::Begin("Saros", &state.showExplorer);
        app.run();

        ImGui::End();
    }


    if (state.showWaveformEditor) {

        ImGui::SetNextWindowSize(ImVec2(512, 0), ImGuiCond_Once);
        if (!ImGui::Begin("Waveforms", &state.showWaveformEditor)) {
            state.showWaveformEditor = false;
        }

        static float duration = 5.0f;
        static uint32_t maxFreq = 3000;
        if (ImGui::Button("Play")) {

            synth.PlayVoice(63, 0, 0.5f, duration, Fractonica::Synth::OscSine, ModulateTone);
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            synth.StopVoice(63);
        }

        bool changed = false;
        ImGui::SameLine();
        uint32_t maxFreqBounds = 100000;
        uint32_t zero = 0;
        ImGui::SliderFloat("Duration", &duration, 0.0f, 100.0f);
        ImGui::SliderScalarN("Frequency", ImGuiDataType_U32, &maxFreq, 1, &zero, &maxFreqBounds);
        changed |= ImGui::SliderFloat("Frequency", &state.frequency, 0.0f, 1000.0f);
        changed |= ImGui::SliderFloat("Amp", &state.amp, 0.0f, 500.0f);
        changed |= ImGui::SliderFloat("Offset", &state.offset, 0.0f, 500.0f);

        if (changed)   tone_generator.Randomize(state.frequency, state.amp);

        ImGui::Separator();
        static constexpr int waveCount = 1024;
        static float wave[waveCount] = {};
        static float phaseAcc = 0;
        phaseAcc+=delta_time;
        float min = 0;
        float max = 0;

        for (int i = 0; i < waveCount; ++i) {
            float v = 0;

            for (int j = 0; j < tone_generator.GetCount(); ++j) {
                Fractonica::Tone* t = tone_generator.Get(i);
                v += std::sin(((float) i / waveCount) * t->phase_inc + t->phase_acc + phaseAcc) * t->amp;
            }
            wave[i] = v;
            if (v < min) min = v;
            if (v > max) max = v;
        }

        ImGui::PlotLines("Form", wave, waveCount, 0, 0, min, max, ImVec2(512, 128));

        if (ImGui::BeginChild("##s")) {
            for (int i = 0; i < tone_generator.GetCount(); ++i) {

                char n[8];
                sprintf(n, "%d", i);

                Fractonica::Tone* t = tone_generator.Get(i);
                if (ImGui::TreeNode(n)) {
                    ImGui::SliderScalarN("Frequency", ImGuiDataType_U32, &t->phase_inc, 1, &zero, &maxFreq);
                    ImGui::SliderInt("Amp", &t->amp, 0, 100);
                   // ImGui::SliderScalarN("Phase", ImGuiDataType_U32, &t->phase_acc, 0, M_PI * 2);
                    ImGui::TreePop();
                }
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }



    sg_pass render_pass{};
    render_pass.action = state.pass_action;
    render_pass.swapchain = sglue_swapchain();
    render_pass.label = "render-pass";

    sg_begin_pass(&render_pass);

    solid_explorer.draw();

    simgui_render();


    sg_end_pass();
    sg_commit();

}

void input(const sapp_event* ev) {
    simgui_handle_event(ev);
}

void cleanup() {
    app.shutdown();
    solid_explorer.shutdown();
   // state.mandelbrot.shutdown();
    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();


}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = 2000,
        .height = 1200,
        .window_title = "Fractonica",
        .logger.func = slog_func,
    };
}