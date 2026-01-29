#ifndef MEGA2560BOOT_H
#define MEGA2560BOOT_H

#include <LunarClockApp.h>
#include <ILI9481Display.h>
#include <DS3231UnixClock.h>
#include <ArduinoInput.h>

Fractonica::ArduinoInput input;
Fractonica::IDisplay *display = new Fractonica::ILI9481Display();
Fractonica::IUnixClock *unixClock = new Fractonica::DS3231UnixClock();
Fractonica::LunarClockApp app(display, unixClock, &input);

void setup()
{
    Wire.begin();
    app.setup();
}

uint64_t prevMicros = 0;

void loop()
{
    const uint64_t m = micros();
    const int32_t dt = m - prevMicros;
    app.loop(dt);
    prevMicros = m;
}

#endif