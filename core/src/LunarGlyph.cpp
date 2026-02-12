//
// Created by Dmitry Popov on 30.01.2026.
//

#include <LunarGlyph.h>
#include <LunarTime.h>

namespace Fractonica {

    static constexpr uint16_t bit4(int x, int y) { return static_cast<uint16_t>(1u) << (y * 4 + x); }

    static constexpr uint16_t STEP_MASKS[] = {
        bit4(2,1),                         // base (always)
        bit4(1,1),                         // n>0
        bit4(1,2),                         // n>1
        bit4(2,2),                         // n>2
        bit4(3,0)|bit4(3,1)|bit4(3,2),     // n>3
        bit4(0,0)|bit4(1,0)|bit4(2,0),     // n>4
        bit4(0,1)|bit4(0,2)|bit4(0,3),     // n>5
        bit4(1,3)|bit4(2,3)|bit4(3,3),     // n>6
    };

    static uint16_t glyphMask(uint8_t n) {
        if (n > 7) n = 7;
        uint16_t m = 0;
        for (uint8_t i = 0; i <= n; ++i) m |= STEP_MASKS[i];
        return m;
    }

    enum class QuadOp : uint8_t {
        Identity = 0,
        Rot90 = 1,
        Rot180 = 2,
        Rot270 = 3,
        MirrorX = 4,
        MirrorY = 5,
        MirrorDiag = 6,
        MirrorAntiDiag = 7
    };

    static constexpr QuadOp SYMMETRIC_OPERATIONS[12][4] = {
    {QuadOp::Identity, QuadOp::MirrorX, QuadOp::Rot90, QuadOp::MirrorDiag},
    {QuadOp::Identity, QuadOp::MirrorX, QuadOp::Rot180, QuadOp::MirrorY},
    {QuadOp::Identity, QuadOp::MirrorX, QuadOp::Rot270, QuadOp::MirrorAntiDiag},
    {QuadOp::Rot90, QuadOp::MirrorDiag, QuadOp::Identity, QuadOp::MirrorX},
    {QuadOp::Rot90, QuadOp::MirrorDiag, QuadOp::Rot180, QuadOp::MirrorY},
    {QuadOp::Rot90, QuadOp::MirrorDiag, QuadOp::Rot270, QuadOp::MirrorAntiDiag},
    {QuadOp::Rot180, QuadOp::MirrorY, QuadOp::Identity, QuadOp::MirrorX},
    {QuadOp::Rot180, QuadOp::MirrorY, QuadOp::Rot90, QuadOp::MirrorDiag},
    {QuadOp::Rot180, QuadOp::MirrorY, QuadOp::Rot270, QuadOp::MirrorAntiDiag},
    {QuadOp::Rot270, QuadOp::MirrorAntiDiag, QuadOp::Identity, QuadOp::MirrorX},
    {QuadOp::Rot270, QuadOp::MirrorAntiDiag, QuadOp::Rot90, QuadOp::MirrorDiag},
    {QuadOp::Rot270, QuadOp::MirrorAntiDiag, QuadOp::Rot180, QuadOp::MirrorY}
    };

    static void applyOp4(uint8_t x, uint8_t y, QuadOp op, uint8_t &ox, uint8_t &oy) {
        switch (op) {
            case QuadOp::Identity:      ox=x;      oy=y;      break;
            case QuadOp::Rot90:         ox=3-y;    oy=x;      break;
            case QuadOp::Rot180:        ox=3-x;    oy=3-y;    break;
            case QuadOp::Rot270:        ox=y;      oy=3-x;    break;
            case QuadOp::MirrorX:       ox=3-x;    oy=y;      break;
            case QuadOp::MirrorY:       ox=x;      oy=3-y;    break;
            case QuadOp::MirrorDiag:    ox=y;      oy=x;      break;
            case QuadOp::MirrorAntiDiag:ox=3-y;    oy=3-x;    break;
        }
    }

    static constexpr uint8_t ORGX[4] = {0, 4, 4, 0};
    static constexpr uint8_t ORGY[4] = {0, 0, 4, 4};

    static void drawGlyph(uint8_t n, uint8_t d, uint32_t color, QuadOp op, IMatrix *matrix)
    {
        d &= 3;
        const uint8_t baseX = ORGX[d];
        const uint8_t baseY = ORGY[d];

        const uint16_t m = glyphMask(n);

        for (uint8_t y = 0; y < 4; ++y) {
            for (uint8_t x = 0; x < 4; ++x) {
                if (m & (static_cast<uint16_t>(1u) << (y*4 + x))) {
                    uint8_t tx, ty;
                    applyOp4(x, y, op, tx, ty);
                    matrix->drawPixel(baseX + tx, baseY + ty, color);
                }
            }
        }
    }

    static void drawGlyph(uint8_t n, uint8_t d, uint32_t color, QuadOp op,
                       int16_t x, int16_t y, uint8_t size, IDisplay *display)
    {
        d &= 3;
        const uint8_t baseX = ORGX[d]; // quad origin in 8x8 cell space
        const uint8_t baseY = ORGY[d];

        const uint16_t m = glyphMask(n);

        for (uint8_t gy = 0; gy < 4; ++gy) {
            for (uint8_t gx = 0; gx < 4; ++gx) {

                // bit index in 4x4 mask (row-major)
                const uint8_t bit = (gy * 4u) + gx;
                if ((m & (uint16_t(1u) << bit)) == 0) continue;

                // transform within the 4x4 quad
                uint8_t tx, ty;
                applyOp4(gx, gy, op, tx, ty);

                // map to absolute cell in the 8x8 grid
                const uint8_t cellX = uint8_t(baseX + tx); // 0..7
                const uint8_t cellY = uint8_t(baseY + ty); // 0..7

                // map cell to pixels
                const int16_t px = int16_t(x + int16_t(cellX) * int16_t(size));
                const int16_t py = int16_t(y + int16_t(cellY) * int16_t(size));

                display->drawRect(px, py, size, size, color);
            }
        }
    }

    void LunarGlyph::drawRange(const int64_t now, const int64_t from, const int64_t to, IMatrix *matrix) const {

        if (now > to || now < from) return;
        matrix->clear();

        const auto bin = static_cast<int32_t>(floor( 4096 * ((static_cast<double>(now) - static_cast<double>(from))
            / (static_cast<double>(to) - static_cast<double>(from)))));
        const auto sym = SYMMETRIC_OPERATIONS[0];
        for (int8_t i = 3; i >= 0; i--) {
            drawGlyph((bin >> 3 * i) % 8, 3 - i, 0xFF0000, sym[i], matrix);
        }
        matrix->flush();
    }

    void LunarGlyph::draw(const int64_t timestamp, IMatrix *matrix) const {

        matrix->clear();

        const auto newMoon = time_.getEventInfo(timestamp, NEW_MOON);
        const auto node = time_.getEventInfo(timestamp, NODAL_ASCENDING);
        const auto apogee = time_.getEventInfo(timestamp, APOGEE);

        const uint32_t color = matrix->getColorHSV((apogee.bin + 2048) % 4096 * 16, 255, 255);

        drawGlyph((newMoon.bin >> 9) % 8, 0, color, static_cast<QuadOp>((node.bin >> 0) % 8), matrix);
        drawGlyph((newMoon.bin >> 6) % 8, 1, color, static_cast<QuadOp>((node.bin >> 3) % 8), matrix);
        drawGlyph((newMoon.bin >> 3) % 8, 2, color, static_cast<QuadOp>((node.bin >> 6) % 8), matrix);
        drawGlyph((newMoon.bin >> 0) % 8, 3, color, static_cast<QuadOp>((node.bin >> 9) % 8), matrix);

        matrix->flush();
    }



    void LunarGlyph::draw(uint32_t *lastNewMoon, uint32_t *lastNode, uint32_t *lastApogee, int64_t timestamp, int16_t x, int16_t y, uint8_t size,  IDisplay *display) const {

        const auto newMoon = time_.getEventInfo(timestamp, NEW_MOON);
        const auto node = time_.getEventInfo(timestamp, NODAL_ASCENDING);
        const auto apogee = time_.getEventInfo(timestamp, APOGEE);

        if (*lastNewMoon == newMoon.bin && *lastNode == node.bin && *lastApogee == apogee.bin) {
            return;
        }

        *lastNewMoon = newMoon.bin;
        *lastNode = node.bin;
        *lastApogee = apogee.bin;

        display->drawRect(x, y, 8 * size, 8 * size, 0x0000);

     //

        const uint32_t color = apogee.bin * 8;

        drawGlyph((newMoon.bin >> 9) % 8, 0, color, static_cast<QuadOp>((node.bin >> 0) % 8), x, y, size, display);
        drawGlyph((newMoon.bin >> 6) % 8, 1, color, static_cast<QuadOp>((node.bin >> 3) % 8), x, y, size,display);
        drawGlyph((newMoon.bin >> 3) % 8, 2, color, static_cast<QuadOp>((node.bin >> 6) % 8), x, y, size,display);
        drawGlyph((newMoon.bin >> 0) % 8, 3, color, static_cast<QuadOp>((node.bin >> 9) % 8), x, y, size,display);
    }
}
