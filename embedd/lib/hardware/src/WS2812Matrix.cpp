//
// Created by Dmitry Popov on 29.01.2026.
//
#include "WS2812Matrix.h"
#include "Adafruit_NeoPixel.h"

namespace Fractonica {

    uint16_t WS2812Matrix::getPixelIndex(const uint16_t x, const uint16_t y) const {
        const uint16_t physicalRow = height - 1 - y;
        return (x * width) + physicalRow;
    }

    void WS2812Matrix::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
        strip.setPixelColor(getPixelIndex(x,y), color);

    }

    bool WS2812Matrix::begin() {

        if (strip.begin()) {
            strip.setBrightness(brightness);
            strip.show();
            return true;
        }
        return false;
    }

    uint32_t WS2812Matrix::getColor(const uint8_t r, const uint8_t g, const uint8_t b) const {
        return Adafruit_NeoPixel::Color(r, g, b);
    }

    uint32_t WS2812Matrix::getColorHSV(uint16_t h, uint8_t s, uint8_t v) const {
        return Adafruit_NeoPixel::ColorHSV(h, s, v);
    }

    void WS2812Matrix::flush() {
        strip.show();
    }

    void WS2812Matrix::clear() {
        strip.clear();
    }
}
