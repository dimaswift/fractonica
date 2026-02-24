//
// Created by Dmitry Popov on 20.02.2026.
//
#include "../include/OctalGlyph.h"
#include "../include/Utils.h"
#include "../include/Vector2.h"
#define BITS 12

const static Vector2 diamond[BITS] = {
    Vector2(2,2),
    Vector2(3,1),
    Vector2(4,2),
    Vector2(5,3),
    Vector2(6,4),
    Vector2(5,5),
    Vector2(4,6),
    Vector2(3,7),
    Vector2(2,6),
    Vector2(1,5),
    Vector2(0,4),
    Vector2(1,3)
};

const static Vector2 innerDiamond[4] = {
    Vector2(3,3),
    Vector2(4,4),
    Vector2(3,5),
    Vector2(2,4)
};

static uint64_t toOctal(const uint64_t decimalNumber) {
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

static void drawDiamond(Fractonica::IDisplay *draw_list, Vector2 center, const Fractonica::OctalGlyphSettings &settings) {
    const auto p0 = Vector2(center.x - settings.size, center.y);
    const auto p1 = Vector2(center.x + settings.size, center.y);
    const auto p2 = Vector2(center.x, center.y + settings.size);
    const auto p3 = Vector2(center.x, center.y - settings.size);
    draw_list->drawLine(p0, p2, settings.thickness, settings.color);
    draw_list->drawLine(p2, p1, settings.thickness, settings.color);
    draw_list->drawLine(p1, p3, settings.thickness, settings.color);
    draw_list->drawLine(p0, p3, settings.thickness, settings.color);
}

void Fractonica::OctalGlyph::Draw(const uint64_t &value, IDisplay *display, const Vector2& position, const OctalGlyphSettings &settings) {

    int drawn = 0;
    auto limit = settings.symbolLimit;
    Vector2 p = position;
    uint64_t v = value;

    uint8_t size = 0;
    uint64_t v0 = value;

    while (v0 != 0) {
        size++;
        v0 >>= 12;
    }

    while (size > limit) {
        v >>= 12;
        size--;
    }
    const uint64_t originalValue = v;
    while (v != 0) {
        const auto center = Vector2(p.x + settings.size * 3 + settings.size, p.y + settings.size * 4);
        drawDiamond(display, center, settings);
        drawn++;
        for (int i = 0; i < 4; ++i) {
            const auto anchor = Vector2(p.x + innerDiamond[i].x * settings.size + settings.size, p.y + innerDiamond[i].y * settings.size);
            Vector2 prev = anchor;
            for (int j = 0; j < 3; ++j) {
                const bool bit = v >> ((3 * i) + j) & 1;
                const auto d = diamond[i * 3 + j];
                switch (settings.type) {
                    case Pixel:
                        if (bit)
                            display->drawNGonFilled(Vector2(p.x + d.x * settings.size + settings.size,p.y + d.y * settings.size), settings.size, settings.color, 4);
                        display->drawNGonFilled(anchor, settings.size, settings.color, 4);
                        break;
                    case Line:
                        if (bit) {
                            display->drawLine(anchor, Vector2(p.x + d.x * settings.size + settings.size, p.y + d.y * settings.size), settings.thickness, settings.color);
                        }
                        break;
                    case Path:
                        const auto next = Vector2(p.x + d.x * settings.size + settings.size, p.y + d.y * settings.size);
                        if (bit) {
                            display->drawLine(prev, next, settings.thickness, settings.color);
                            prev = next;
                        }
                        break;
                }
            }
        }

        v >>= 12;

        if (v == 0) break;

        if (settings.horizontal) {
            p.x += settings.size * 6;
        }
        else {
            p.y += settings.size * 6;
        }
    }

    const auto step = settings.size * 8;
    const auto pad = settings.size * 0.25f + 2;
    const auto corner = settings.size * 8 + (((float)drawn - 1) * settings.size * 6);
    auto end = position;

    if (settings.horizontal) {
        end.y += step;
        if (settings.showBorder) {
            display->drawRect(Vector2(position.x,position.y), Vector2(position.x + corner,position.y + step), settings.color);
        }
    }
    else {
        end.y += corner;
        if (settings.showBorder) {
            display->drawRect(Vector2(position.x,position.y), Vector2(position.x + step, position.y + corner), settings.color);
        }
    }
    const auto rect = settings.horizontal ? Vector2(corner,step) : Vector2(step, corner);
    display->expand(rect.x, rect.y);
     if (settings.showLabels) {
         constexpr auto textH = 14.0f;
         display->setCursor(Vector2(position.x + pad,position.y  + pad));
         display->printF("%lld", toOctal(originalValue));
         auto tPos = Vector2(position.x + pad, position.y + corner - pad - textH);
         if (settings.horizontal) {
             tPos = Vector2(position.x + pad, position.y + step - pad - textH);
         }
         display->setCursor(tPos);
         display->printF("%lld", originalValue);
     }
     display->setCursor(end);
}