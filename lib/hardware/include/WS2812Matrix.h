#ifndef WS2812MATRIX_H
#define WS2812MATRIX_H

#include <IMatrix.h>

#include "Adafruit_NeoPixel.h"

namespace Fractonica {

    class WS2812Matrix : public IMatrix {

        Adafruit_NeoPixel strip;
        uint16_t brightness, width, height;
        uint16_t getPixelIndex(uint16_t x, uint16_t y) const;

    public:
        WS2812Matrix(const uint8_t pin, const uint16_t brightness, const uint16_t width, const uint16_t height)
        :

        strip(width * height, pin, NEO_GRB + NEO_KHZ800) {
            this->brightness = brightness;
            this->width = width;
            this->height = height;
        }

        void drawPixel(uint16_t x, uint16_t y, uint32_t color) override;
        bool begin() override;
        uint32_t getColor(uint8_t r, uint8_t g, uint8_t b)  const override;
        uint32_t getColorHSV(uint16_t h, uint8_t s, uint8_t v)  const override;
        ~WS2812Matrix() override = default;

        void flush() override;
        void clear() override;
    };
}
#define WS2812MATRIX_H
#endif
