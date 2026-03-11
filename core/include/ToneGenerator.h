//
// Created by Dmitry Popov on 11.03.2026.
//

#ifndef FRACTONICA_TONEGENERATOR_H
#define FRACTONICA_TONEGENERATOR_H

#include <cmath>
#include "SineLUT.h"

namespace Fractonica {

    struct Tone {
        uint32_t phase_inc;
        uint32_t phase_acc;
        int32_t amp;

    };

    class ToneGenerator {
        Tone* tones;
        int capacity;
        int count = 0;
        int sampleRate;

    public:

        ToneGenerator(const int capacity_, const int sampleRate_) : capacity(capacity_), sampleRate(sampleRate_) {
            tones = new Tone[capacity];
        }

        ~ToneGenerator() {
            delete[] tones;
        }


        void Add(float freq, float amp) {
            if (count >= capacity) return;

            tones[count].phase_inc = static_cast<uint32_t>((freq / sampleRate) * 4294967296.0);
            tones[count].phase_acc = 0;

            tones[count].amp = static_cast<int32_t>(amp * 256.0f);

            count++;
        }

        [[nodiscard]] int GetCount() const {
            return count;
        }

        void Randomize(const float frequency, const float amp) {
            count = 0;
            for (int i = 0; i < capacity; ++i) {
                Add(std::sin(i * frequency * 100) * amp, 100.0f - i);
            }
        }

        Tone* Get(int index) {
            if (index >= count) return nullptr;
            return &tones[index];
        }

        void ModulateFast(uint32_t phase, int32_t base_freq_fixed, int32_t base_vol_fixed, uint32_t duration_samples, int32_t& out_freq, int32_t& out_vol) {

            int32_t v = base_freq_fixed;

            for (int i = 0; i < count; ++i) {
                Tone& t = tones[i];

                uint32_t table_idx =  (phase * t.phase_inc) >> SINELUT_PHASE_SHIFT;

                int32_t sine_val = sine_lut.data[table_idx];

                v += (sine_val * t.amp) >> 8;
            }

            out_freq = v;

            if (phase < duration_samples) {
                int32_t half_vol = base_vol_fixed >> 1;
                int32_t decay_amount = (base_vol_fixed * phase) / duration_samples;
                out_vol = half_vol - decay_amount;
                if (out_vol < 0) out_vol = 0;
            } else {
                out_vol = 0;
            }

        }
    };

}

#endif