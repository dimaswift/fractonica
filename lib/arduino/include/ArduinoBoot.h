#pragma once
#include <Arduino.h>
#include "DS1307UnixClock.h"
#include "LunarGlyph.h"
#include "WS2812Matrix.h"

static Fractonica::DS1307UnixClock unixClock;
static Fractonica::WS2812Matrix matrix(7, 150, 8, 8);
static Fractonica::LunarGlyph glyph;

void setup()
{
    Serial.begin(9600);

    if (!unixClock.begin()) {
        Serial.println("Failed to initialize clock");
    }

    matrix.begin();
}

void loop()
{
    glyph.draw(unixClock.now(), &matrix);
}