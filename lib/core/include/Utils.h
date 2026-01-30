//
// Created by Dmitry Popov on 30.01.2026.
//

#ifndef FRACTONICA_UTILS_H
#define FRACTONICA_UTILS_H
#include <stdint.h>

namespace Fractonica {

    class Utils {
    public:
        static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
        static uint32_t ColorHSV(uint16_t hue, uint8_t sat, uint8_t val);
    };

    inline uint32_t Utils::Color(uint8_t r, uint8_t g, uint8_t b) {
        return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }

    inline uint32_t Utils::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {
        uint8_t r, g, b;

        hue = (hue * 1530L + 32768) / 65536;

        if (hue < 510) {
            b = 0;
            if (hue < 255) {
                r = 255;
                g = hue;       //     g = 0 to 254
            } else {         //   Yellow to Green-1
                r = 510 - hue; //     r = 255 to 1
                g = 255;
            }
        } else if (hue < 1020) { // Green to Blue-1
            r = 0;
            if (hue < 765) { //   Green to Cyan-1
                g = 255;
                b = hue - 510;  //     b = 0 to 254
            } else {          //   Cyan to Blue-1
                g = 1020 - hue; //     g = 255 to 1
                b = 255;
            }
        } else if (hue < 1530) { // Blue to Red-1
            g = 0;
            if (hue < 1275) { //   Blue to Magenta-1
                r = hue - 1020; //     r = 0 to 254
                b = 255;
            } else { //   Magenta to Red-1
                r = 255;
                b = 1530 - hue; //     b = 255 to 1
            }
        } else { // Last 0.5 Red (quicker than % operator)
            r = 255;
            g = b = 0;
        }

        const uint32_t v1 = 1 + val;
        const uint16_t s1 = 1 + sat;
        const uint8_t s2 = 255 - sat;
        return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
             (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
             (((((b * s1) >> 8) + s2) * v1) >> 8);
    }
}

#endif //FRACTONICA_UTILS_H