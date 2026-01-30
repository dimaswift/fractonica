
#ifndef CORE_IDISPLAY_H
#define CORE_IDISPLAY_H

#include <stdint.h>
#include <Vector2.h>

#include "IMatrix.h"

namespace Fractonica
{
    class IDisplay : public IMatrix
    {
    public:
        virtual ~IDisplay() = default;
        virtual void print(const char *msg, int16_t x, int16_t y, uint8_t size) = 0;
        virtual void log(const char *msg) = 0;
        virtual void logError(const char *msg)= 0;
        virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) = 0;
        virtual void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) = 0;
        virtual void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) = 0;
        virtual void update() = 0;
        virtual bool isOpen() = 0;
        virtual Vector2 size() = 0;
    };
}

#endif