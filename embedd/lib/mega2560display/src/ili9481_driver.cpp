#include "ili9481_driver.h"
#include <Arduino.h>
#include <math.h>

void ILI9481Driver::init(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset) {
    hw.init(cs, cd, wr, rd, reset);
    hw.reset();
    delay(200);

    rotation = 0;
    width = ILI9481_WIDTH;
    height = ILI9481_HEIGHT;

    cursorX = 0;
    cursorY = 0;
    textColor = 0xFFFF;
    textBgColor = 0x0000;
    textSize = 1;
    textWrap = true;

    initRegisters();
    setRotation(0);
    invertDisplay(false);
}

void ILI9481Driver::initRegisters() {
    hw.setCS(true);

    hw.writeCmd(ILI9481_SLPOUT);
    hw.setCS(false);
    delay(120);
    hw.setCS(true);

    hw.writeCmd(0xD0);
    hw.writeData(0x07);
    hw.writeData(0x42);
    hw.writeData(0x18);

    hw.writeCmd(0xD1);
    hw.writeData(0x00);
    hw.writeData(0x07);
    hw.writeData(0x10);

    hw.writeCmd(0xD2);
    hw.writeData(0x01);
    hw.writeData(0x02);

    hw.writeCmd(0xC0);
    hw.writeData(0x10);
    hw.writeData(0x3B);
    hw.writeData(0x00);
    hw.writeData(0x02);
    hw.writeData(0x11);

    hw.writeCmd(0xC5);
    hw.writeData(0x03);

    hw.writeCmd(0xC8);
    hw.writeData(0x00);
    hw.writeData(0x32);
    hw.writeData(0x36);
    hw.writeData(0x45);
    hw.writeData(0x06);
    hw.writeData(0x16);
    hw.writeData(0x37);
    hw.writeData(0x75);
    hw.writeData(0x77);
    hw.writeData(0x54);
    hw.writeData(0x0C);
    hw.writeData(0x00);

    hw.writeCmd(ILI9481_MADCTL);
    hw.writeData(0x0A);

    hw.writeCmd(ILI9481_PIXFMT);
    hw.writeData(0x55);

    hw.writeCmd(ILI9481_CASET);
    hw.writeData(0x00);
    hw.writeData(0x00);
    hw.writeData(0x01);
    hw.writeData(0x3F);

    hw.writeCmd(ILI9481_PASET);
    hw.writeData(0x00);
    hw.writeData(0x00);
    hw.writeData(0x01);
    hw.writeData(0xE0);

    hw.setCS(false);
    delay(120);
    hw.setCS(true);

    hw.writeCmd(ILI9481_DISPON);

    hw.setCS(false);
}

void ILI9481Driver::setRotation(uint8_t rot) {
    rotation = rot & 3;

    uint8_t madctl = 0;
    switch (rotation) {
        case 0:
            madctl = 0x09;
            width = ILI9481_WIDTH;
            height = ILI9481_HEIGHT;
            break;
        case 1:
            madctl = 0x2B;
            width = ILI9481_HEIGHT;
            height = ILI9481_WIDTH;
            break;
        case 2:
            madctl = 0x0A;
            width = ILI9481_WIDTH;
            height = ILI9481_HEIGHT;
            break;
        case 3:
            madctl = 0x28;
            width = ILI9481_HEIGHT;
            height = ILI9481_WIDTH;
            break;
        default: break;
    }

    hw.setCS(true);
    hw.writeCmdData8(ILI9481_MADCTL, madctl);
    hw.setCS(false);
}

void ILI9481Driver::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    hw.writeCmd(ILI9481_CASET);
    hw.writeData(x0 >> 8);
    hw.writeData(x0 & 0xFF);
    hw.writeData(x1 >> 8);
    hw.writeData(x1 & 0xFF);

    hw.writeCmd(ILI9481_PASET);
    hw.writeData(y0 >> 8);
    hw.writeData(y0 & 0xFF);
    hw.writeData(y1 >> 8);
    hw.writeData(y1 & 0xFF);
}

void ILI9481Driver::invertDisplay(bool invert) {
    hw.setCS(true);
    hw.writeCmd(invert ? ILI9481_INVON : ILI9481_INVOFF);
    hw.setCS(false);
}

void ILI9481Driver::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= width || y >= height) return;
    if (x + w > width) w = width - x;
    if (y + h > height) h = height - y;

    hw.setCS(true);
    setWindow(x, y, x + w - 1, y + h - 1);
    hw.writeCmd(ILI9481_RAMWR);

    uint32_t count = static_cast<uint32_t>(w) * h;
    while (count--) {
        hw.writeData16(color);
    }

    hw.setCS(false);
}

void ILI9481Driver::fillScreen(const uint16_t color) {
    fillRect(0, 0, width, height, color);
}

void ILI9481Driver::drawBuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* buffer) {
    if (x >= width || y >= height || !buffer) return;
    if (x + w > width) w = width - x;
    if (y + h > height) h = height - y;

    hw.setCS(true);
    setWindow(x, y, x + w - 1, y + h - 1);
    hw.writeCmd(ILI9481_RAMWR);

    const uint32_t count = static_cast<uint32_t>(w) * h;
    for (uint32_t i = 0; i < count; i++) {
        hw.writeData16(buffer[i]);
    }

    hw.setCS(false);
}

void ILI9481Driver::drawBuffer565(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* buffer) {
    drawBuffer(x, y, w, h, buffer);
}

void ILI9481Driver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const uint16_t color) {
    if (x0 == x1) {
        if (y0 > y1) { auto t = y0; y0 = y1; y1 = t; }
        if (x0 < 0 || x0 >= static_cast<int16_t>(width)) return;
        if (y1 < 0 || y0 >= static_cast<int16_t>(height)) return;
        if (y0 < 0) y0 = 0;
        if (y1 >= static_cast<int16_t>(height)) y1 = height - 1;
        fillRect(x0, y0, 1, y1 - y0 + 1, color);
        return;
    }

    if (y0 == y1) {
        if (x0 > x1) { int16_t t = x0; x0 = x1; x1 = t; }
        if (y0 < 0 || y0 >= static_cast<int16_t>(height)) return;
        if (x1 < 0 || x0 >= static_cast<int16_t>(width)) return;
        if (x0 < 0) x0 = 0;
        if (x1 >= static_cast<int16_t>(width)) x1 = width - 1;
        fillRect(x0, y0, x1 - x0 + 1, 1, color);
        return;
    }

    const int16_t dx = abs(x1 - x0);
    const int16_t dy = abs(y1 - y0);
    const int16_t sx = x0 < x1 ? 1 : -1;
    const int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;

    hw.setCS(true);
    while (true) {
        if (x0 >= 0 && x0 < static_cast<int16_t>(width) && y0 >= 0 && y0 < static_cast<int16_t>(height)) {
            setWindow(x0, y0, x0, y0);
            hw.writeCmd(ILI9481_RAMWR);
            hw.writeData16(color);
        }

        if (x0 == x1 && y0 == y1) break;

        const int16_t e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    hw.setCS(false);
}

void ILI9481Driver::drawThickLine(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const uint16_t thickness, const uint16_t color) {
    if (thickness == 1) {
        drawLine(x0, y0, x1, y1, color);
        return;
    }

    const int16_t dx = x1 - x0;
    const int16_t dy = y1 - y0;
    const float len = sqrt(dx * dx + dy * dy);
    if (len < 0.001f) {
        fillRect(x0 - thickness/2, y0 - thickness/2, thickness, thickness, color);
        return;
    }

    const float nx = -dy / len;
    const float ny = dx / len;
    const int16_t offset = thickness / 2;

    const int16_t x0a = x0 + nx * offset;
    const int16_t y0a = y0 + ny * offset;
    const int16_t x0b = x0 - nx * offset;
    const int16_t y0b = y0 - ny * offset;
    const int16_t x1a = x1 + nx * offset;
    const int16_t y1a = y1 + ny * offset;
    const int16_t x1b = x1 - nx * offset;
    const int16_t y1b = y1 - ny * offset;

    const int16_t points[8] = {x0a, y0a, x1a, y1a, x1b, y1b, x0b, y0b};
    fillPolygon(points, 4, color);
}

void ILI9481Driver::drawSegments(const int16_t* points, uint16_t count, uint16_t color) {
    if (count < 2) return;

    for (uint16_t i = 0; i < count - 1; i++) {
        int16_t x0 = points[i * 2];
        int16_t y0 = points[i * 2 + 1];
        int16_t x1 = points[(i + 1) * 2];
        int16_t y1 = points[(i + 1) * 2 + 1];
        drawLine(x0, y0, x1, y1, color);
    }
}

void ILI9481Driver::drawThickSegments(const int16_t* points, uint16_t count, uint16_t thickness, uint16_t color) {
    if (count < 2) return;

    for (uint16_t i = 0; i < count - 1; i++) {
        drawThickLine(
            points[i * 2], points[i * 2 + 1],
            points[(i + 1) * 2], points[(i + 1) * 2 + 1],
            thickness, color
        );
    }
}

void ILI9481Driver::fillPolygon(const int16_t* points, uint16_t count, uint16_t color) {
    if (count < 3) return;

    int16_t minY = points[1], maxY = points[1];
    for (uint16_t i = 1; i < count; i++) {
        int16_t y = points[i * 2 + 1];
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }

    if (minY < 0) minY = 0;
    if (maxY >= height) maxY = height - 1;

    hw.setCS(true);
    for (int16_t y = minY; y <= maxY; y++) {
        int16_t intersections[32];
        uint16_t intersectionCount = 0;

        for (uint16_t i = 0; i < count; i++) {
            uint16_t j = (i + 1) % count;
            int16_t x0 = points[i * 2];
            int16_t y0 = points[i * 2 + 1];
            int16_t x1 = points[j * 2];
            int16_t y1 = points[j * 2 + 1];

            if ((y0 <= y && y < y1) || (y1 <= y && y < y0)) {
                int16_t x = x0 + (int32_t)(y - y0) * (x1 - x0) / (y1 - y0);
                if (intersectionCount < 32) {
                    intersections[intersectionCount++] = x;
                }
            }
        }

        for (uint16_t i = 0; i < intersectionCount - 1; i++) {
            for (uint16_t j = i + 1; j < intersectionCount; j++) {
                if (intersections[i] > intersections[j]) {
                    int16_t temp = intersections[i];
                    intersections[i] = intersections[j];
                    intersections[j] = temp;
                }
            }
        }

        for (uint16_t i = 0; i < intersectionCount; i += 2) {
            if (i + 1 < intersectionCount) {
                int16_t x0 = intersections[i];
                int16_t x1 = intersections[i + 1];

                if (x0 < 0) x0 = 0;
                if (x1 >= width) x1 = width - 1;

                if (x0 <= x1) {
                    setWindow(x0, y, x1, y);
                    hw.writeCmd(ILI9481_RAMWR);
                    for (int16_t x = x0; x <= x1; x++) {
                        hw.writeData16(color);
                    }
                }
            }
        }
    }
    hw.setCS(false);
}

void ILI9481Driver::drawPolygon(const int16_t* points, const uint16_t count, const uint16_t color) {
    if (count < 2) return;

    for (uint16_t i = 0; i < count; i++) {
        uint16_t j = (i + 1) % count;
        drawLine(points[i * 2], points[i * 2 + 1], points[j * 2], points[j * 2 + 1], color);
    }
}

void ILI9481Driver::fillAndDrawPolygon(const int16_t* points, const uint16_t count, const uint16_t fillColor, const uint16_t outlineColor) {
    fillPolygon(points, count, fillColor);
    drawPolygon(points, count, outlineColor);
}
#include "ili9481_font.h"

void ILI9481Driver::drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if ((x >= static_cast<int16_t>(width)) || (y >= static_cast<int16_t>(height)) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0)) {
        return;
    }

    auto ch = static_cast<uint8_t>(c);
    if (ch >= 176) ch++;

    hw.setCS(true);
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t line = (i < 5) ? pgm_read_byte(font5x7 + (ch * 5) + i) : 0x00;

        for (uint8_t j = 0; j < 8; j++) {
            if (line & 0x1) {
                if (size == 1) {
                    if (x + i >= 0 && x + i < static_cast<int16_t>(width) && y + j >= 0 && y + j < static_cast<int16_t>(height)) {
                        setWindow(x + i, y + j, x + i, y + j);
                        hw.writeCmd(ILI9481_RAMWR);
                        hw.writeData16(color);
                    }
                } else {
                    fillRect(x + i * size, y + j * size, size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) {
                    if (x + i >= 0 && x + i < static_cast<int16_t>(width) && y + j >= 0 && y + j < static_cast<int16_t>(height)) {
                        setWindow(x + i, y + j, x + i, y + j);
                        hw.writeCmd(ILI9481_RAMWR);
                        hw.writeData16(bg);
                    }
                } else {
                    fillRect(x + i * size, y + j * size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
    hw.setCS(false);
}

void ILI9481Driver::drawString(const char* str, const int16_t x, const int16_t y) {
    setCursor(x, y);
    while (*str) {
        write(*str++);
    }
}

void ILI9481Driver::drawStringCentered(const char* str, const int16_t y) {
    const uint16_t len = strlen(str) * 6 * textSize;
    const int16_t x = (width - len) / 2;
    drawString(str, x, y);
}

void ILI9481Driver::setCursor(const int16_t x, const int16_t y) {
    cursorX = x;
    cursorY = y;
}

void ILI9481Driver::setTextColor(const uint16_t color) {
    textColor = textBgColor = color;
}

void ILI9481Driver::setTextColor(const uint16_t color, const uint16_t bg) {
    textColor = color;
    textBgColor = bg;
}

void ILI9481Driver::setTextSize(const uint8_t size) {
    textSize = (size > 0) ? size : 1;
}

void ILI9481Driver::setTextWrap(const bool wrap) {
    textWrap = wrap;
}

size_t ILI9481Driver::write(const uint8_t c) {
    if (c == '\n') {
        cursorY += textSize * 8;
        cursorX = 0;
    } else if (c == '\r') {

    } else {
        if (textWrap && (cursorX + textSize * 6 > static_cast<int16_t>(width))) {
            cursorX = 0;
            cursorY += textSize * 8;
        }
        drawChar(cursorX, cursorY, c, textColor, textBgColor, textSize);
        cursorX += textSize * 6;
    }
    return 1;
}

size_t ILI9481Driver::print(const char* str) {
    size_t n = 0;
    while (*str) {
        write(*str++);
        n++;
    }
    return n;
}
