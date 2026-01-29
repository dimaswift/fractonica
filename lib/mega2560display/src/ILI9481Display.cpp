#include "ILI9481Display.h"

namespace Fractonica
{
    bool ILI9481Display::isOpen()
    {
        return true;
    }

    void ILI9481Display::update()
    {
    }

    void ILI9481Display::log(const char *msg)
    {
        driver.setTextColor(WHITE);
        driver.setTextSize(textSize);
        driver.print(msg);
    }

    Vector2 ILI9481Display::size()
    {
        return Vector2{.x = 320, .y = 480};
    }

    void ILI9481Display::print(const char *msg, int16_t x, int16_t y, uint8_t size)
    {
        driver.setTextColor(WHITE);
        driver.setTextSize(size);
        driver.setCursor(x,y);
        driver.print(msg);
    }

    void ILI9481Display::logError(const char *msg)
    {
        driver.setTextSize(textSize);
        driver.setTextColor(RED);
        driver.print(msg);
    }

    void ILI9481Display::flush()
    {
    }

    void ILI9481Display::clear()
    {
        driver.fillScreen(BLACK);
    }

    void ILI9481Display::drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap)
    {
        driver.drawBuffer565(x, y, width, height, bitmap);
    }

    void ILI9481Display::drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        driver.drawPixel(x, y, color);
    }

    void ILI9481Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
    {
        driver.drawLine(x0, y0, x1, y1, color);
    }

    bool ILI9481Display::begin()
    {
        driver.init(40, 38, 39, 255, 41);
        driver.fillScreen(BLACK);
        textSize = 2;
        return true;
    }
};

