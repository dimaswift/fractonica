//
// Created by Dmitry Popov on 20.02.2026.
//

#ifndef FRACTONICA_GLYPH_H
#define FRACTONICA_GLYPH_H
#include "imgui.h"

namespace Fractonica {

    enum GlyphType {
        Pixel = 0,
        Line = 1,
        Path = 2
      };

    class Glyph {
        int current = 0;
        double time = 0;
        GlyphType selectedType = Path;
        int page = 0;
        int rowSize = 8;
        float size = 28;
        float thickness = 3;
        ImColor color = ImColor(255, 255, 255);
        bool showLabels = false;
        bool showSettings = false;
        int saros = 141;

        void draw(int64_t value, ImDrawList* draw_list, ImVec2 position, GlyphType type) const;

    public:
        Glyph(int saros_) : saros(saros_) {}
        void render(double dt);
    };
}


#endif //FRACTONICA_GLYPH_H