#pragma once
#include "IMatrix.h"

#include <vector>
#include "imgui.h"

#include "IDisplay.h"

namespace Fractonica {

    class ImGuiDisplay : public IDisplay {
    public:
        ImGuiDisplay(uint16_t width,
                    uint16_t height,
                    float scale = 10.0f,
                    const char* windowName = "Matrix");

        // IMatrix
        void drawPixel(uint16_t x, uint16_t y, uint32_t color) override;
        bool begin() override;
        void flush() override;
        void clear() override;
        [[nodiscard]] uint32_t getColor(uint8_t r, uint8_t g, uint8_t b) const override;
        [[nodiscard]] uint32_t getColorHSV(uint16_t h, uint8_t s, uint8_t v) const override;
        [[nodiscard]] uint16_t width()  const { return width_; }
        [[nodiscard]] uint16_t height() const { return height_; }
        void print(const char *msg, int16_t x, int16_t y, uint8_t size) override;
        void log(const char *msg) override;
        void logError(const char *msg) override;
        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t thickness, uint32_t color) override;
        void drawLine(const Vector2& p1, const Vector2& p2, int16_t thickness, uint32_t color) override;
        void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) override;
        void drawFillRect(const Vector2& min, const Vector2& max, uint32_t color) override;
        void drawNGonFilled(const Vector2& center, float radius, uint32_t col, int num_segments) override;
        void drawRect(const Vector2& min, const Vector2& max, uint32_t color) override;
        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;
        void setCursor(const Vector2& pos) override;
        void printF(const char* msg, ...) override;
        void expand(int16_t w, int16_t h) override;
        void update() override;
        bool isOpen() override;
        Vector2 size() override;
    private:
        uint16_t width_;
        uint16_t height_;
        float scale_;
        const char* windowName_;
        std::vector<uint32_t> fb_; // row-major: y*width + x
        bool begun_ = false;
        static ImU32 toImU32(uint32_t rgb);
    };
} // namespace Fractonica