//
// Created by Dmitry Popov on 10.03.2026.
//

#ifndef EMBEDD_SSD1306DISPLAY_H
#define EMBEDD_SSD1306DISPLAY_H
#include <IDisplay.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_SSD1306.h"

namespace Fractonica {
    class SSD1306Display : public IDisplay {

        Adafruit_SSD1306 display;
        int sclPin;
        int sdaPin;
        int screenAddress;
    public:

        SSD1306Display(const int width, const int height, const int sclPin, const int sdaPin, const int screenAddress) :
        sclPin(sclPin),
        sdaPin(sdaPin),
        screenAddress(screenAddress),
        display(width, height, &Wire, -1) {}

        void drawPixel(uint16_t x, uint16_t y, uint32_t color) override;

        bool begin() override;

        void flush() override;

        void clear() override;

        [[nodiscard]] uint32_t getColor(uint8_t r, uint8_t g, uint8_t b) const override;

        [[nodiscard]] uint32_t getColorHSV(uint16_t h, uint8_t s, uint8_t v) const override;

        ~SSD1306Display() override {
        }

        void print(const char *msg, int16_t x, int16_t y, uint8_t size) override;

        void log(const char *msg) override;

        void logError(const char *msg) override;

        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t thickness, uint32_t color) override;

        void drawLine(const Vector2 &p1, const Vector2 &p2, int16_t thickness, uint32_t color) override;

        void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) override;

        void drawFillRect(const Vector2 &min, const Vector2 &max, uint32_t color) override;

        void drawNGonFilled(const Vector2 &center, float radius, uint32_t col, int num_segments) override;

        void drawRect(const Vector2 &min, const Vector2 &max, uint32_t color) override;

        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;

        void setCursor(const Vector2 &pos) override;

        void expand(int16_t w, int16_t h) override;

        void printF(const char *fmt, ...) override;

        void update() override;

        bool isOpen() override;

        Vector2 size() override;
    };

    inline void SSD1306Display::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
        display.drawPixel(x, y, color);
    }

    inline bool SSD1306Display::begin() {
        Wire.begin(sdaPin , sclPin);
        if (!display.begin(SSD1306_SWITCHCAPVCC, screenAddress)) {
            return false;
        }
        display.setRotation(3);
        display.clearDisplay();
        return true;
    }

    inline void SSD1306Display::flush() {
        display.display();
    }

    inline void SSD1306Display::clear() {
        display.clearDisplay();
    }

    inline uint32_t SSD1306Display::getColor(uint8_t r, uint8_t g, uint8_t b) const {
        return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b;
    }

    inline uint32_t SSD1306Display::getColorHSV(uint16_t h, uint8_t s, uint8_t v) const {
        return 0xFFFFFFFF;
    }

    inline void SSD1306Display::print(const char *msg, int16_t x, int16_t y, uint8_t size) {
        display.setTextSize(size);
        display.setTextColor(WHITE);
        display.setCursor(x, y);
        display.print(msg);
    }

    inline void SSD1306Display::log(const char *msg) {
        print(msg, 0, 0, 2);
    }

    inline void SSD1306Display::logError(const char *msg) {
        print(msg, 0, 0, 1);
    }

    inline void SSD1306Display::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t thickness,
        uint32_t color) {
        display.drawLine(x1, y1, x2, y2, WHITE);
    }

    inline void SSD1306Display::drawLine(const Vector2 &p1, const Vector2 &p2, int16_t thickness, uint32_t color) {
        display.drawLine(p1.x, p1.y, p2.x, p2.y, WHITE);
    }

    inline void SSD1306Display::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
        display.drawRect(x, y, w, h, WHITE);
    }

    inline void SSD1306Display::drawFillRect(const Vector2 &min, const Vector2 &max, uint32_t color) {
        display.fillRect(min.x, min.y, max.x - min.x, max.y - min.y, WHITE);
    }

    inline void SSD1306Display::drawNGonFilled(const Vector2 &center, float radius, uint32_t col, int num_segments) {

    }

    inline void SSD1306Display::drawRect(const Vector2 &min, const Vector2 &max, uint32_t color) {
        display.drawRect(min.x, min.y, max.x - min.x, max.y - min.y, WHITE);
    }

    inline void SSD1306Display::drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height,
                                           const uint16_t *bitmap) {
        display.drawGrayscaleBitmap(x,y, (uint8_t*) bitmap, width, height);
    }

    inline void SSD1306Display::setCursor(const Vector2 &pos) {
        display.setCursor(pos.x, pos.y);
    }

    inline void SSD1306Display::expand(int16_t w, int16_t h) {
    }

    inline void SSD1306Display::printF(const char *fmt, ...) {
        display.setTextColor(WHITE);
        display.setTextSize(1);
        va_list args;
        va_start(args, fmt);
        display.printf(fmt, args);
        va_end(args);
    }

    inline void SSD1306Display::update() {

    }

    inline bool SSD1306Display::isOpen() {
        return true;
    }

    inline Vector2 SSD1306Display::size() {
        return Vector2(display.width(), display.height());
    }
}

#endif //EMBEDD_SSD1306DISPLAY_H