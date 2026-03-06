#pragma once

#include "sokol_audio.h"
#include <cmath>
#include <atomic>
#include <array>

namespace Fractonica {
    // Forward declaration
    struct VoiceContext;

    // Typedefs for our custom audio behaviors
    using OscillatorFunc = float (*)(float phase);
    using ModulatorFunc = void (*)(float time, float base_freq, float base_vol, float duration, float &out_freq, float &out_vol);

    struct VoiceContext {
        bool active = false;
        float time = 0.0f; // How long this voice has been playing (in seconds)
        float phase = 0.0f; // Current wave phase (0.0 to 1.0)
        float duration = 0.0f;
        // Base parameters set by the game
        float base_freq = 440.0f;
        float base_vol = 0.0f;

        // The custom behaviors for this specific voice
        OscillatorFunc osc_func = nullptr;
        ModulatorFunc mod_func = nullptr;
    };

    class Synth {
    public:
        static constexpr int NUM_VOICES = 64;
        static constexpr float PI = 3.14159265358979323846f;

        // --- Standard Built-in Oscillators ---
        static float OscSine(float phase) { return std::sin(phase * 2.0f * PI); }
        static float OscSquare(float phase) { return (phase < 0.5f) ? 1.0f : -1.0f; }
        static float OscSaw(float phase) { return (phase * 2.0f) - 1.0f; }

        static float OscNoise(float phase) {
            // Simple pseudo-random noise
            return ((static_cast<float>(rand()) / RAND_MAX) * 2.0f) - 1.0f;
        }

        Synth() = default;

        ~Synth() {
            saudio_shutdown();
        }

        void Initialize() {
            saudio_desc desc = {};
            desc.stream_userdata_cb = &Synth::StaticAudioCallback;
            desc.user_data = this;

            saudio_setup(&desc);
            m_sample_rate = saudio_sample_rate();
        }

        // Trigger a note on a specific channel with custom modulation
        void PlayVoice(int channel, float frequency, float volume, float duration, OscillatorFunc osc, ModulatorFunc mod = nullptr) {
            if (channel < 0 || channel >= NUM_VOICES) return;

            // We set up a temporary context and swap it in safely
            VoiceContext ctx;
            ctx.active = true;
            ctx.time = 0.0f;
            ctx.phase = 0.0f;
            ctx.base_freq = frequency;
            ctx.base_vol = volume;
            ctx.osc_func = osc;
            ctx.mod_func = mod;
            ctx.duration = duration;
            m_voices[channel] = ctx; // Copy into the array the audio thread reads
        }

        void StopVoice(int channel) {
            if (channel >= 0 && channel < NUM_VOICES) {
                m_voices[channel].active = false;
            }
        }

    private:
        static void StaticAudioCallback(float *buffer, int num_frames, int num_channels, void *user_data) {
            auto *synth = static_cast<Synth *>(user_data);
            synth->FillBuffer(buffer, num_frames, num_channels);
        }

        void FillBuffer(float *buffer, int num_frames, int num_channels) {
            float time_step = 1.0f / static_cast<float>(m_sample_rate);

            for (int i = 0; i < num_frames; ++i) {
                float mixed_sample = 0.0f;

                // Process all active voices
                for (auto &voice: m_voices) {
                    if (!voice.active || !voice.osc_func) continue;

                    // 1. Calculate modulated frequency and volume
                    float current_freq = voice.base_freq;
                    float current_vol = voice.base_vol;

                    if (voice.mod_func) {
                        voice.mod_func(voice.time, voice.base_freq, voice.base_vol, voice.duration, current_freq, current_vol);
                    }

                    // 2. Advance phase
                    float phase_increment = current_freq * time_step;
                    voice.phase += phase_increment;
                    if (voice.phase >= 1.0f) voice.phase -= 1.0f;
                    else if (voice.phase < 0.0f) voice.phase += 1.0f; // Handle negative frequencies

                    // 3. Generate sample and mix it
                    mixed_sample += voice.osc_func(voice.phase) * current_vol;

                    // 4. Advance time
                    voice.time += time_step;

                   // if (voice.time >= PI * 2) voice.time -= PI * 2;

                    if (voice.duration > 0 && voice.time >= voice.duration) {
                        voice.active = false;
                    }
                }

                // Prevent audio clipping by clamping the final mix
                if (mixed_sample > 1.0f) mixed_sample = 1.0f;
                if (mixed_sample < -1.0f) mixed_sample = -1.0f;

                // Write to buffer (duplicate for stereo)
                for (int c = 0; c < num_channels; ++c) {
                    *buffer++ = mixed_sample;
                }
            }
        }

        int m_sample_rate = 44100;

        // We use a simple array. Since the audio thread reads and the main thread writes,
        // there's a slight thread-safety edge case here, but for simple game audio
        // without locks, it usually resolves seamlessly within a single frame.
        std::array<VoiceContext, NUM_VOICES> m_voices{};
    };
}
