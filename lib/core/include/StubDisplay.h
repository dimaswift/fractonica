//
// Created by Dmitry Popov on 29.01.2026.
//

#ifndef FRACTONICA_STUBDISPLAY_H
#define FRACTONICA_STUBDISPLAY_H
#include "IDisplay.h"

namespace Fractonica {

    class StubDisplay : public IDisplay {
    public:
        ~StubDisplay() override;

        bool begin() override;

        void print(const char *msg, int16_t x, int16_t y, uint8_t size) override;

        void log(const char *msg) override;

        void logError(const char *msg) override;

        void flush() override;

        void clear() override;

        void drawPixel(int16_t x, int16_t y, uint16_t color) override;

        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;

        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;

        void update() override;

        bool isOpen() override;

        Vector2 size() override;
    };

    inline StubDisplay::~StubDisplay() = default;

    inline bool StubDisplay::begin() {
        return true;
    }

    inline void StubDisplay::print(const char *msg, int16_t x, int16_t y, uint8_t size) {
    }

    inline void StubDisplay::log(const char *msg) {
    }

    inline void StubDisplay::logError(const char *msg) {
    }

    inline void StubDisplay::flush() {
    }

    inline void StubDisplay::clear() {
    }

    inline void StubDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
    }

    inline void StubDisplay::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    }

    inline void StubDisplay::drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) {
    }

    inline void StubDisplay::update() {
    }

    inline bool StubDisplay::isOpen() {
        return true;
    }

    inline Vector2 StubDisplay::size() {
        return Vector2{.x = 0, .y = 0};
    }
}


#endif //FRACTONICA_STUBDISPLAY_H