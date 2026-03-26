//
// Created by Dmitry Popov on 11.03.2026.
//

#ifndef FRACTONICA_SYNTH_H
#define FRACTONICA_SYNTH_H

#include "SineLUT.h"

namespace Fractonica {

    using OscillatorFunc = int16_t (*)(uint32_t phase);

    using ModulatorFunc = void (*)(uint32_t sample_counter, uint32_t base_phase_inc, int32_t base_vol, uint32_t duration_samples, int32_t &out_phase_inc, int32_t &out_vol);

    struct VoiceContext {
        bool active = false;

        uint32_t sample_counter = 0;
        uint32_t duration_samples = 0;

        uint32_t phase = 0;
        int32_t base_phase_inc = 0;
        int32_t base_vol = 0;

        OscillatorFunc osc_func = nullptr;
        ModulatorFunc mod_func = nullptr;
    };

    class Synth {
        static constexpr int NUM_VOICES = 8;
        int sampleRate;
        VoiceContext m_voices[NUM_VOICES] = {};

    public:

        int GetCapacity() {return NUM_VOICES;}

        static int16_t OscSine(uint32_t phase) {
            return sine_lut.data[phase >> SINELUT_PHASE_SHIFT];
        }

        static int16_t OscSquare(uint32_t phase) {
            return (phase & 0x80000000) ? -32767 : 32767;
        }

        static int16_t OscSaw(uint32_t phase) {
            return static_cast<int16_t>((phase >> 16) - 32768);
        }

        static int16_t OscNoise(uint32_t phase) {
            static uint32_t seed = 123456789;
            seed = (1103515245 * seed + 12345) & 0x7FFFFFFF;
            return static_cast<int16_t>((seed >> 15) - 32768);
        }

        Synth(const int sampleRate_ = 44100) : sampleRate(sampleRate_) {}

        void SetSampleRate(const int rate) {
            sampleRate = rate;
        }

        void PlayVoice(const int channel, const float frequency, const float volume, const float duration, const OscillatorFunc osc, const ModulatorFunc mod = nullptr) {
            if (channel < 0 || channel >= NUM_VOICES) return;

            VoiceContext ctx;
            ctx.active = true;
            ctx.sample_counter = 0;
            ctx.phase = 0;

            ctx.base_phase_inc = static_cast<int32_t>((frequency / sampleRate) * 4294967296.0);
            ctx.base_vol = static_cast<int32_t>(volume * 256.0f); // 256 = 1.0 volume
            ctx.duration_samples = static_cast<uint32_t>(duration * sampleRate);

            ctx.osc_func = osc;
            ctx.mod_func = mod;

            m_voices[channel] = ctx;
        }

        void StopVoice(int channel) {
            if (channel >= 0 && channel < NUM_VOICES) {
                m_voices[channel].active = false;
            }
        }

        int16_t Sample() {
            int32_t mixed_sample = 0;

            for (auto &voice: m_voices) {
                if (!voice.active || !voice.osc_func) continue;

                int32_t current_phase_inc = voice.base_phase_inc;
                int32_t current_vol = voice.base_vol;

                if (voice.mod_func) {
                    voice.mod_func(voice.sample_counter, voice.base_phase_inc, voice.base_vol, voice.duration_samples, current_phase_inc, current_vol);
                }

                int32_t osc_val = voice.osc_func(voice.phase);

                mixed_sample += (osc_val * current_vol) >> 8;

                voice.phase += current_phase_inc;
                voice.sample_counter++;

                if (voice.duration_samples > 0 && voice.sample_counter >= voice.duration_samples) {
                    voice.active = false;
                }
            }

            mixed_sample = mixed_sample >> 2;

            if (mixed_sample > 32767) mixed_sample = 32767;
            if (mixed_sample < -32768) mixed_sample = -32768;

            return static_cast<int16_t>(mixed_sample);
        }
    };
}

#endif //FRACTONICA_SYNTH_H