#include <Arduino.h>
#include "DS1307UnixClock.h"
#include "LunarGlyph.h"
#include "WS2812Matrix.h"

static auto unixClock = new Fractonica::DS1307UnixClock();
static auto matrix = new Fractonica::WS2812Matrix(7, 150, 8, 8);
static Fractonica::LunarGlyph glyph;

void setup()
{
    Serial.begin(9600);

    if (!unixClock->begin()) {
        Serial.println("Failed to initialize clock");
    }

    matrix->begin();
}

void loop()
{
    glyph.draw(unixClock->now(), matrix);
}