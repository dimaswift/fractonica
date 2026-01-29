#ifndef BASE8HEPTCLOCK_H
#define BASE8HEPTCLOCK_H

#include <stdint.h>
#include <IDisplay.h>

namespace Fractonica
{
    class Base8HeptClock
    {
    public:
        struct Config
        {
            uint8_t numRings = 3;
            uint8_t ringWidth = 16;
            uint16_t colorWhite = 0xFFFF;
            uint16_t colorBlack = 0x0000;
        };
        Config config;
        Base8HeptClock(uint8_t numRings, uint8_t ringWidth, IDisplay *display)
            : display(display)
        {
            screenHeight = display->size().y;
            screenWidth = display->size().x;
            config.numRings = numRings;
            config.ringWidth = ringWidth;
        }

        void draw(uint32_t currentCounter, uint32_t &lastCounter, uint8_t orientation, int16_t x, int16_t y);

    private:
        struct Point
        {
            int16_t x, y;
        };

        static constexpr uint8_t kMaxRings = 8;

        uint16_t screenWidth, screenHeight;
        Point ringBoundaryVerts_[kMaxRings][7]{};
        IDisplay *display;

        static int32_t edgeFunction(int16_t ax, int16_t ay,
                             int16_t bx, int16_t by,
                             int16_t px, int16_t py) ;

        void fillTriangle(int16_t x0, int16_t y0,
                          int16_t x1, int16_t y1,
                          int16_t x2, int16_t y2,
                          uint16_t color) const;

        void fillQuad(Point p0, Point p1, Point p2, Point p3,
                      uint16_t color) const;

        static Point getHeptagonVertex(uint8_t index, uint16_t radius, uint8_t orientation, int16_t x, int16_t y) ;

    

    
    };

} // namespace Fractonica

#endif