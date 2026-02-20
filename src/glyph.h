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
        GlyphType selectedType = Line;
        int page = 0;
        int rowSize = 8;
        float size = 10;
        float thickness = 2;
        ImColor color = ImColor(255, 255, 255);
        bool showLabels = true;
        void draw(int value, ImDrawList* draw_list, ImVec2 position, GlyphType type) const;
    public:
        void render(double dt);
    };
}


#endif //FRACTONICA_GLYPH_H