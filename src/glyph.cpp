//
// Created by Dmitry Popov on 20.02.2026.
//
#include "glyph.h"

#include <string>

#include "Utils.h"
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

void Fractonica::Glyph::draw(int value, ImDrawList *draw_list, ImVec2 position, GlyphType type) const {

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
        ImGui::Text("d%d", value);
        ImGui::Text("o%d", toOctal(value));
        ImGui::Text("b%s", bin);
        ImGui::EndTooltip();
    }
    draw_list->AddRect(ImVec2(position.x,position.y), ImVec2(position.x + size * 8,position.y + size * 8), color);

    if (showLabels) {

        ImGui::SetCursorScreenPos(ImVec2(position.x + 4,position.y + 4));
        ImGui::TextColored(color,"%04d", toOctal(value));

        ImGui::SetCursorScreenPos(ImVec2(position.x + size * 5 - 4 , position.y + 4));
        ImGui::TextColored(color,"%04d", value);

        ImGui::SetCursorScreenPos(ImVec2(position.x + 4 , position.y + size * 6 + 2));
        ImGui::TextColored(color,"%X", value);

        ImGui::SetCursorScreenPos(ImVec2(position.x + size * 7 - 4 , position.y + size * 6 + 2));
        ImGui::TextColored(color,"%c", base64[value % 64]);
    }

    ImGui::SetCursorScreenPos(ImVec2(position.x,position.y + size * 9));
}



void Fractonica::Glyph::render(double dt) {

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(1024, 1024), ImGuiCond_Once);

    ImGui::Begin("Glyph");

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    time += dt;

    if (time >= 0.1) {
        time = 0;
        current = current + 1;
    }

    static char testValue[5];

    const ImVec2 c = ImGui::GetCursorScreenPos();
    draw(current, draw_list, ImVec2(c.x, c.y), Pixel);
    draw(current, draw_list, ImVec2(c.x + size * 8, c.y), Line);
    draw(current, draw_list, ImVec2(c.x + size * 16, c.y), Path);

    ImGui::InputText("Octal", testValue, 5);
    long value = strtol(testValue, nullptr, 8);
    draw(value, draw_list, ImVec2(c.x, c.y + size * 9), Path);

    ImGui::Dummy(ImVec2(size * 8, size * 8));
    int type = (selectedType);
    ImGui::RadioButton("Pixel", &type, 0); ImGui::SameLine();
    ImGui::RadioButton("Line", &type, 1); ImGui::SameLine();
    ImGui::RadioButton("Path", &type, 2); ImGui::SameLine();
    ImGui::Checkbox("Show Labels", &showLabels);
    selectedType = static_cast<GlyphType>(type);
    ImGui::SliderInt("Page", &page, 0, 4096 / (rowSize * rowSize) - 1);
    ImGui::SliderFloat("Size", &size, 2, 16);
    ImGui::SliderFloat("Thickness", &thickness, 0.5, 8);

    ImGui::Dummy(ImVec2(size, size));
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const ImVec2 center = ImVec2(p.x, p.y);

   // draw_list->AddRectFilled(ImVec2(center.x - size, center.y - size), ImVec2(center.x + size * rowSize * 9, center.y + size * rowSize * 9), ImColor(255, 255, 255));
    for (int y = 0; y < rowSize; ++y) {
        for (int x = 0; x < rowSize; ++x) {
            int value = ((rowSize * rowSize) * page) + x * rowSize + y;
            draw( value, draw_list, ImVec2(center.x + x * size * 9, center.y + y * size * 9), selectedType);

        }
    }

    ImGui::End();
}
