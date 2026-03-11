//
// Created by Dmitry Popov on 11.03.2026.
//
#ifndef FRACTONICA_SINELUT_H
#define FRACTONICA_SINELUT_H

#include <math.h>

constexpr int SINELUT_TABLE_BITS = 10;
constexpr int SINELUT_TABLE_SIZE = 1 << SINELUT_TABLE_BITS;
constexpr int SINELUT_PHASE_SHIFT = 32 - SINELUT_TABLE_BITS; // To shift a 32-bit int down to a 10-bit table index


struct SineTable {
    int16_t data[SINELUT_TABLE_SIZE]{};
    SineTable() {
        for (int i = 0; i < SINELUT_TABLE_SIZE; ++i) {
            data[i] = static_cast<int16_t>(std::sin(static_cast<float>(i) / SINELUT_TABLE_SIZE * 2.0f * M_PI) * 32767.0f);
        }
    }
};

static SineTable sine_lut;

#endif //FRACTONICA_SINELUT_H