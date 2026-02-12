#ifndef DESKTOP_APP
#define DESKTOP_APP

#include "UnixClock.h"
#include "LunarGlyph.h"
#include "ImGuiDisplay.h"
#include "LunarTime.h"
#include "imgui.h"

namespace Fractonica {
    class DesktopApp {
        LunarTime lunarTime;
        UnixClock unixClock;
        ImGuiDisplay matrix;
        LunarGlyph glyph;

        public:
        DesktopApp();
        void run();
        void setup();
        void shutdown();
    };

}


#endif
