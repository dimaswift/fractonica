#ifndef DESKTOP_APP
#define DESKTOP_APP

#include "UnixClock.h"
#include "ImGuiDisplay.h"
#include "LunarTime.h"


namespace Fractonica {
    class DesktopApp {
        LunarTime lunarTime;
        UnixClock unixClock;
        ImGuiDisplay matrix;

        public:
        DesktopApp();
        void run();
        void setup();
        void shutdown();
    };

}


#endif
