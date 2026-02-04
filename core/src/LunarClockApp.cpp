#include "LunarClockApp.h"
#include <new_moon.h>
#include <apogee.h>
#include <nodal_ascending.h>

namespace Fractonica
{
    LunarClockApp::LunarClockApp(IDisplay *display,
                                 IUnixClock *unixClock, IInput *input) : App(display, unixClock, input), clock(4, 12, display)
    {
        fractonica_mem_init(&newMoon, FRACTONICA_NEW_MOON_COUNT, fractonica_new_moon_timestamps);
        fractonica_mem_init(&apogee, FRACTONICA_APOGEE_COUNT, fractonica_apogee_timestamps);
        fractonica_mem_init(&nodalAscending, FRACTONICA_NODAL_ASCENDING_COUNT, fractonica_nodal_ascending_timestamps);
        resolution = static_cast<uint32_t>(pow(8, clock.config.numRings));
    }

    void LunarClockApp::onInput(const InputEvent &event)
    {
        switch (event.type)
        {
        case InputEventType::BUTTON_PRESSED:
            getDisplay()->log("Hello");
            break;

        case InputEventType::VALUE_CHANGED:
            if (event.id == SLIDER_Y)
            {
                posY = static_cast<int16_t>(event.value);
                getDisplay()->clear();
                prevApogee = 0;
                prevNewMoon = 0;
                prevNodalAscending = 0;
                time = 1000000;
            }
            break;

        default:
            break;
        }
    }

    void LunarClockApp::loop(const int32_t dt)
    {
        App::loop(dt);
        time += dt;
        if (time >= 100000)
        {
            time = 0;
            const int64_t now = getUnixClock()->now();
            const fractonica_ephemeris_fraction_t newMoonFr = fractonica_ephemeris_fraction_at(&newMoon, now, resolution, nullptr, nullptr);
            const fractonica_ephemeris_fraction_t apogeeFr = fractonica_ephemeris_fraction_at(&apogee, now, resolution, nullptr, nullptr);
            const fractonica_ephemeris_fraction_t nodal_ascendingFr = fractonica_ephemeris_fraction_at(&nodalAscending, now, resolution, nullptr, nullptr);

            clock.draw(newMoonFr.bin, prevNewMoon, 0, posX, posY);
            clock.draw(apogeeFr.bin, prevApogee, 1, posX - offsetX, posY - offsetY);
            clock.draw(nodal_ascendingFr.bin, prevNodalAscending, 1, posX + offsetX, posY - offsetY);
            clock.draw(now % resolution, prevTime, 1, posX, posY - 120);
        }

        getDisplay()->update();
    }
}