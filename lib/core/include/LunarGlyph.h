//
// Created by Dmitry Popov on 30.01.2026.
//

#ifndef FRACTONICA_LUNARGLYPH_H
#define FRACTONICA_LUNARGLYPH_H

#include "IDisplay.h"
#include "IMatrix.h"
#include "LunarTime.h"

namespace Fractonica {
    class LunarGlyph {

        LunarTime time_;
    public:

        LunarGlyph() : time_(LunarTime(4, 8)) {}
        void draw(int64_t timestamp, IMatrix *matrix) const;
        void draw(uint32_t *lastNewMoon, uint32_t *lastNode, uint32_t *lastApogee, int64_t timestamp, int16_t x, int16_t y, uint8_t size, IDisplay *display) const;
    };
}

#endif //FRACTONICA_LUNARGLYPH_H