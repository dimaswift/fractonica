#ifndef ILI9481_DRIVER_H
#define ILI9481_DRIVER_H

#include <stdint.h>
#include "hw_interface.h"

#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#else
 #define PROGMEM
#endif

#define ILI9481_WIDTH 320
#define ILI9481_HEIGHT 480

#define ILI9481_SLPOUT 0x11
#define ILI9481_MADCTL 0x36
#define ILI9481_PIXFMT 0x3A
#define ILI9481_CASET 0x2A
#define ILI9481_PASET 0x2B
#define ILI9481_RAMWR 0x2C
#define ILI9481_RAMRD 0x2E
#define ILI9481_DISPON 0x29
#define ILI9481_INVOFF 0x20
#define ILI9481_INVON 0x21

#define ILI9481_MADCTL_MY 0x80
#define ILI9481_MADCTL_MX 0x40
#define ILI9481_MADCTL_MV 0x20
#define ILI9481_MADCTL_ML 0x10
#define ILI9481_MADCTL_BGR 0x08

class ILI9481Driver {
private:
    HardwareInterface hw;
    uint16_t width, height;
    uint8_t rotation;
    int16_t cursorX, cursorY;
    uint16_t textColor, textBgColor;
    uint8_t textSize;
    bool textWrap;

    void initRegisters();

public:
    void init(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset);
    void setRotation(uint8_t rot);
    void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void invertDisplay(bool invert);

    inline void drawPixel(uint16_t x, uint16_t y, uint16_t color) __attribute__((always_inline)) {
        if (x >= width || y >= height) return;
        hw.setCS(true);
        setWindow(x, y, x, y);
        hw.writeCmd(ILI9481_RAMWR);
        hw.writeData16(color);
        hw.setCS(false);
    }

    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawBuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* buffer);
    void drawBuffer565(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* buffer);

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t thickness, uint16_t color);
    void drawSegments(const int16_t* points, uint16_t count, uint16_t color);
    void drawThickSegments(const int16_t* points, uint16_t count, uint16_t thickness, uint16_t color);
    void fillPolygon(const int16_t* points, uint16_t count, uint16_t color);
    void drawPolygon(const int16_t* points, uint16_t count, uint16_t color);
    void fillAndDrawPolygon(const int16_t* points, uint16_t count, uint16_t fillColor, uint16_t outlineColor);

    void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
    void drawString(const char* str, int16_t x, int16_t y);
    void drawStringCentered(const char* str, int16_t y);
    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t color);
    void setTextColor(uint16_t color, uint16_t bg);
    void setTextSize(uint8_t size);
    void setTextWrap(bool wrap);
    size_t write(uint8_t c);
    size_t print(const char* str);

    inline uint16_t getWidth() const { return width; }
    inline uint16_t getHeight() const { return height; }
    inline uint8_t getRotation() const { return rotation; }

    inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) __attribute__((always_inline)) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
};

#endif
