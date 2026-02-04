#pragma once
#include "IMatrix.h"

#include <vector>
#include "imgui.h"

#include "IDisplay.h"

namespace Fractonica {
    enum class LedShape { Square, Circle };

    class ImGuiDisplay : public IDisplay {
    public:
        ImGuiDisplay(uint16_t width,
                    uint16_t height,
                    float scale = 10.0f,
                    LedShape shape = LedShape::Circle,
                    const char* windowName = "Matrix");

        void setScale(float s) { scale_ = (s < 1.0f) ? 1.0f : s; }
        [[nodiscard]] float scale() const { return scale_; }

        void setShape(LedShape s) { shape_ = s; }
        [[nodiscard]] LedShape shape() const { return shape_; }

        void setWindowName(const char* name) { windowName_ = name ? name : "Matrix"; }

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
        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
        void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) override;
        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;
        void update() override;
        bool isOpen() override;
        Vector2 size() override;
    private:
        uint16_t width_;
        uint16_t height_;
        float scale_;
        LedShape shape_;
        const char* windowName_;
        std::vector<uint32_t> fb_; // row-major: y*width + x
        bool begun_ = false;
        static ImU32 toImU32(uint32_t rgb);
    };
} // namespace Fractonica