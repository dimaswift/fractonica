//
// Created by Dmitry Popov on 24.02.2026.
//

#ifndef FRACTONICA_OCTAL_GLYPH_H
#define FRACTONICA_OCTAL_GLYPH_H

#include <stdint.h>
#include "IDisplay.h"

namespace Fractonica {
    enum OctalGlyphType {
        Pixel = 0,
        Line = 1,
        Path = 2
      };
    struct OctalGlyphSettings {
        bool horizontal = false;
        bool showLabels = false;
        bool showBorder = true;
        OctalGlyphType type = Path;
        uint8_t symbolLimit = 8;
        float size = 16;
        float thickness = 2;
        uint32_t color = 0xFFFFFFFF;
    };
    class OctalGlyph {
    public:
        static void Draw(const uint64_t& value, IDisplay *display, const Vector2& point, const OctalGlyphSettings &settings);
    };
}


#endif //FRACTONICA_OCTAL_GLYPH_H