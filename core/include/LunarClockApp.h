#ifndef LUNARCLOCK_H
#define LUNARCLOCK_H

#include "Base8HeptClock.h"
#include "Ephemeris.h"
#include "App.h"

namespace Fractonica {
    class LunarClockApp : public App
    {
    private:
        Base8HeptClock clock;
        uint32_t time = 0;
        uint32_t prevNewMoon = 0;
        uint32_t prevApogee = 0;
        uint32_t prevNodalAscending = 0;
        uint32_t prevTime = 0;
        const int16_t offsetX = 82;
        const int16_t offsetY = 20;
        int16_t posY = 405;
        const int16_t posX = 160;
        fractonica_mem_source_t newMoon, apogee, nodalAscending;
        uint32_t resolution;
        

    public:
        LunarClockApp(IDisplay *display, IUnixClock *unixClock, IInput *input);
        void loop(int32_t dt) override;
        void onInput(const InputEvent &event) override;
        enum InputID
        {
            BTN_INC_Y = 0,
            BTN_DEC_Y = 1,
            SLIDER_Y = 2,
        };
    };
    
}

#endif