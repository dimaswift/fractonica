//
// Created by Dmitry Popov on 20.02.2026.
//
#include "glyph.h"

#include <string>

#include "Utils.h"
#include "saros.h"
#define BITS 12

static const char base64[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

const static ImVec2 diamond[BITS] = {
    ImVec2(2,2),
    ImVec2(3,1),
    ImVec2(4,2),
    ImVec2(5,3),
    ImVec2(6,4),
    ImVec2(5,5),
    ImVec2(4,6),
    ImVec2(3,7),
    ImVec2(2,6),
    ImVec2(1,5),
    ImVec2(0,4),
    ImVec2(1,3)
};

const static ImVec2 innerDiamond[4] = {
    ImVec2(3,3),
    ImVec2(4,4),
    ImVec2(3,5),
    ImVec2(2,4)
};

const static uint8_t path[4] = {
    0,1,2,3
};


uint64_t toOctal(uint64_t decimalNumber) {
    uint64_t octalNumber = 0;
    uint64_t i = 1;
    uint64_t tempDecimal = decimalNumber;
    while (tempDecimal != 0)
    {
        uint64_t remainder = tempDecimal % 8;
        octalNumber += remainder * i;
        tempDecimal /= 8;
        i *= 10;
    }
    return octalNumber;
}

void Fractonica::Glyph::draw(int64_t value, ImDrawList *draw_list, ImVec2 position, GlyphType type) const {

    char bin[13];
    bin[12] = '\0';

    ImVec2 center = ImVec2(position.x + size * 3 + size, position.y + size * 4);
    draw_list->AddNgon(center, size, color, 4, thickness);
    for (int i = 0; i < 4; ++i) {
        ImVec2 anchor = ImVec2(position.x + innerDiamond[i].x * size + size, position.y + innerDiamond[i].y * size);
        ImVec2 prev = anchor;
        for (int j = 0; j < 3; ++j) {
            bool bit = value >> ((3 * i) + j) & 1;
            ImVec2 d = diamond[i * 3 + j];

            bin[i * 3 + j] = bit ? '1' : '0';

            switch (type) {
                case Pixel:
                    if (bit)
                        draw_list->AddNgonFilled(ImVec2(position.x + d.x * size + size,position.y + d.y * size), size, color, 4);
                    draw_list->AddNgonFilled(anchor, size, color, 4);
                    break;
                case Line:
                    if (bit) {
                        draw_list->AddLine(anchor, ImVec2(position.x + d.x * size + size, position.y + d.y * size), color, thickness);
                    }
                   // draw_list->AddLine(center, anchor, color, thickness);
                    break;
                case Path:
                    ImVec2 next = ImVec2(position.x + d.x * size + size, position.y + d.y * size);
                    if (bit) {
                        draw_list->AddLine(prev, next, color, thickness);
                        //draw_list->AddCircleFilled(next, size / 3, color);
                       // draw_list->AddLine(center, anchor, color, thickness);
                        prev = next;
                    }



                    break;

            }
        }
    }

    draw_list->AddRect(ImVec2(position.x,position.y), ImVec2(position.x + size * 8,position.y + size * 8), color);
    ImGui::SetCursorScreenPos(ImVec2(position.x,position.y));
    ImGui::Dummy(ImVec2(size * 9, size * 9));
    if (ImGui::BeginItemTooltip()) {
        ImGui::Text("d%lld", value);
        ImGui::Text("o%llu", toOctal(value));
        ImGui::Text("b%s", bin);
        ImGui::EndTooltip();
    }
    draw_list->AddRect(ImVec2(position.x,position.y), ImVec2(position.x + size * 8,position.y + size * 8), color);

    if (showLabels) {

        ImGui::SetCursorScreenPos(ImVec2(position.x + 4,position.y + 4));
        ImGui::TextColored(color,"%012llu", toOctal(value));

        ImGui::SetCursorScreenPos(ImVec2(position.x + size * 5 - 4 , position.y + 4));
        ImGui::TextColored(color,"%012lld", value);

        ImGui::SetCursorScreenPos(ImVec2(position.x + 4 , position.y + size * 7 + 8));
        ImGui::TextColored(color,"%X", value);

        ImGui::SetCursorScreenPos(ImVec2(position.x + size * 8 - 12 , position.y + size * 7 + 8));
        ImGui::TextColored(color,"%c", base64[value % 64]);
    }

   // ImGui::SetCursorScreenPos(ImVec2(position.x,position.y + size * 9));
}



void Fractonica::Glyph::render(double dt) {

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(460, 200), ImGuiCond_Once);
    char name[8];
    sprintf(name, "%03d", saros);
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoScrollbar);


    if (showSettings) {
        if (!ImGui::Begin("Settings", &showSettings, ImGuiWindowFlags_AlwaysAutoResize)) {
            showSettings = false;
        }
        int type = (selectedType);
        ImGui::RadioButton("Pixel", &type, 0); ImGui::SameLine();
        ImGui::RadioButton("Line", &type, 1); ImGui::SameLine();
        ImGui::RadioButton("Path", &type, 2); ImGui::SameLine();
        ImGui::Checkbox("Show Labels", &showLabels);
        ImGui::SliderInt("Page", &page, 0, 4096 / (rowSize * rowSize) - 1);
        ImGui::SliderFloat("Size", &size, 2, 32);
        ImGui::SliderFloat("Thickness", &thickness, 0.5, 8);
        selectedType = static_cast<GlyphType>(type);
        ImGui::End();
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto now = std::time(nullptr);

    auto solarWin = find_solar_saros_window(now, saros);
    auto period = solarWin.future.unix_time * 1000 - solarWin.past.unix_time * 1000;
    auto left = solarWin.future.unix_time * 1000 - now * 1000;
    auto sarosNormalized= static_cast<long double>(left) / static_cast<long double>(period);
    auto sarosA = floor(4096 * (1.0 - sarosNormalized));
    auto sarosB = floor(16777216 * (1.0 - sarosNormalized));
    auto sarosC = floor(68719476736 * (1.0 - sarosNormalized));
    const ImVec2 c = ImGui::GetCursorScreenPos();
    draw(sarosA, draw_list, ImVec2(c.x + size, c.y + size), selectedType);
    draw(sarosB, draw_list, ImVec2(c.x + size * 10, c.y + size), selectedType);
    draw(sarosC, draw_list, ImVec2(c.x + size * 19, c.y + size), selectedType);

    ImGui::Dummy(ImVec2(10, 80));
    if (ImGui::Button("Settings")) {
        showSettings = true;
    }

    ImGui::End();
}
