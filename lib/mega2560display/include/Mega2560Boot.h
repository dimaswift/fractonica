#include <LunarClockApp.h>
#include <ILI9481Display.h>
#include <DS3231UnixClock.h>
#include <ArduinoInput.h>

#include "LunarGlyph.h"

Fractonica::ArduinoInput input;
Fractonica::ILI9481Display display;
Fractonica::DS3231UnixClock unixClock;
Fractonica::LunarGlyph glyph;
Fractonica::LunarClockApp app(&display, &unixClock, &input);

uint32_t lastNewMoon = 0;
uint32_t lastApogee = 0;
uint32_t lastNode = 0;

void setup()
{
    Wire.begin();
    app.setup();

    Serial.begin(9600);

   // unixClock.setUnix(1769739420);
}

uint64_t prevMicros = 0;

void loop()
{
    const uint64_t m = micros();
    const int32_t dt = m - prevMicros;
    app.loop(dt);
    prevMicros = m;

    glyph.draw(&lastNewMoon, &lastNode, &lastApogee, unixClock.now(), 64, 24, 24, &display);
}