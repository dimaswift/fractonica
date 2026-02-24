
#include "Base8HeptClock.h"

#include <math.h>

namespace Fractonica
{
    auto Base8HeptClock::draw(const uint32_t currentCounter, uint32_t &lastCounter, const uint8_t orientation,
                              const int16_t x,
                              const int16_t y) -> void {
        if (config.numRings == 0)
        {
            lastCounter = currentCounter;
            return;
        }

        // 1. Calculate vertices for all rings
        for (uint8_t r = 0; r < config.numRings; r++)
        {
            const uint16_t radius = static_cast<uint16_t>(r + 1) * config.ringWidth;
            for (uint8_t i = 0; i < 7; i++)
            {
                ringBoundaryVerts_[r][i] = getHeptagonVertex(i, radius, orientation, x, y);
            }
        }

        const uint32_t prev = lastCounter;

        // 2. Draw filled sectors (delta update)
        for (uint8_t r = 0; r < config.numRings; r++)
        {
            const uint8_t shift = r * 3;
            const uint8_t curr_d = (currentCounter >> shift) & 0x07;
            const uint8_t prev_d = (prev >> shift) & 0x07;

            // Only redraw fills if value changed
            if (curr_d == prev_d)
                continue;

            const Point *vOuter = ringBoundaryVerts_[r];
            const Point *vInner = (r == 0) ? nullptr : ringBoundaryVerts_[r - 1];

            const bool isRollover = (curr_d < prev_d);

            // Clear old segments if rolling over
            if (isRollover)
            {
                for (uint8_t i = 0; i < prev_d && i < 7; i++)
                {
                    const uint8_t next = (i + 1) % 7;
                    if (r == 0)
                    {
                        fillTriangle(x, y,
                                     vOuter[i].x, vOuter[i].y,
                                     vOuter[next].x, vOuter[next].y,
                                     config.colorBlack);
                    }
                    else if (vInner)
                    {
                        fillQuad(vInner[i], vInner[next], vOuter[next], vOuter[i],
                                 config.colorBlack);
                    }
                }
            }

            // Draw new segments
            const uint8_t start_draw = isRollover ? 0 : prev_d;
            for (uint8_t i = start_draw; i < curr_d && i < 7; i++)
            {
                const uint8_t next = (i + 1) % 7;
                if (r == 0)
                {
                    fillTriangle(x, y,
                                 vOuter[i].x, vOuter[i].y,
                                 vOuter[next].x, vOuter[next].y,
                                 config.colorWhite);
                }
                else if (vInner)
                {
                    fillQuad(vInner[i], vInner[next], vOuter[next], vOuter[i],
                             config.colorWhite);
                }
            }
        }

        // 3. ALWAYS draw the outermost boundary ring
        // Use the vertices calculated in step 1 for the last ring
        Point *vLast = ringBoundaryVerts_[config.numRings - 1];

        for (uint8_t i = 0; i < 7; i++)
        {
            const uint8_t next = (i + 1) % 7;
            display->drawLine(vLast[i].x, vLast[i].y,
                              vLast[next].x, vLast[next].y, 1,
                              config.colorWhite);
        }

        lastCounter = currentCounter;
    }
    

    int32_t Base8HeptClock::edgeFunction(const int16_t ax, const int16_t ay,
                                            const int16_t bx, const int16_t by,
                                            const int16_t px, const int16_t py)
    {
        return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
    }

    void Base8HeptClock::fillTriangle(const int16_t x0, const int16_t y0,
                                        const int16_t x1, const int16_t y1,
                                        const int16_t x2, const int16_t y2,
                                        const uint16_t color) const {
        int16_t minX = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
        int16_t minY = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
        int16_t maxX = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
        int16_t maxY = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);

        if (minX < 0)
            minX = 0;
        if (minY < 0)
            minY = 0;
        if (maxX >=  static_cast<int16_t>(screenWidth))
            maxX = static_cast<int16_t>(screenWidth) - 1;
        if (maxY >=  static_cast<int16_t>(screenHeight))
            maxY = static_cast<int16_t>(screenHeight) - 1;

        for (int16_t y = minY; y <= maxY; y++)
        {
            for (int16_t x = minX; x <= maxX; x++)
            {
                const int32_t w0 = edgeFunction(x1, y1, x2, y2, x, y);
                const int32_t w1 = edgeFunction(x2, y2, x0, y0, x, y);
                const int32_t w2 = edgeFunction(x0, y0, x1, y1, x, y);

                if ((w0 >= 0 && w1 >= 0 && w2 >= 0) ||
                    (w0 <= 0 && w1 <= 0 && w2 <= 0))
                {
                    display->drawPixel(x, y, color);
                }
            }
        }
    }

    auto Base8HeptClock::fillQuad(const Point p0, const Point p1, const Point p2, const Point p3,
                                  const uint16_t color) const -> void {
        fillTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color);
        fillTriangle(p0.x, p0.y, p2.x, p2.y, p3.x, p3.y, color);
    }

    Base8HeptClock::Point Base8HeptClock::getHeptagonVertex(const uint8_t index, const uint16_t radius,
                                                            const uint8_t orientation, const int16_t x,
                                                            int16_t y) {
        constexpr float kPi = 3.14159265359f;
        constexpr float kTwoPi = 6.28318530718f;

        const float theta = -kPi / 2.0f + static_cast<float>(index) * (kTwoPi / 7.0f);
        const float rot = (kPi / 7.0f) * static_cast<float>(orientation);

        Point p{};
        p.x = static_cast<int16_t>(x + static_cast<int16_t>(radius * cosf(theta + rot)));
        p.y = static_cast<int16_t>(y + static_cast<int16_t>(radius * sinf(theta + rot)));
        return p;
    }

} // namespace Fractonica