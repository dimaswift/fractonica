#ifndef ILI9481SHIELD_H

#define ILI9481SHIELD_H

#include <IDisplay.h>
#include "ili9481_driver.h"

#define WIDTH 320
#define HEIGHT 480
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define PIXEL_NUMBER 256

namespace Fractonica {

    class ILI9481Display : public IDisplay
    {
        ILI9481Driver driver;
        uint8_t textSize;

    public:
        ILI9481Display() : driver(), textSize(1) {
        }

        bool isOpen() override;

        void update() override;

        void log(const char *msg) override;

        Vector2 size() override;

        void print(const char *msg, int16_t x, int16_t y, uint8_t size) override;

        void logError(const char *msg) override;

        void flush() override;

        void clear() override;

        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;

        void drawPixel(uint16_t x, uint16_t y, uint32_t color) override;

        void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) override;

        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;

        bool begin() override;

        uint32_t getColor(uint8_t r, uint8_t g, uint8_t b)  const override;

        uint32_t getColorHSV(uint16_t h, uint8_t s, uint8_t v) const override;
    };
}
#endif