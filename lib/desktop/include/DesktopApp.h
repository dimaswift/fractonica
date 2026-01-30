#ifndef DESKTOP_APP
#define DESKTOP_APP

#include "UnixClock.h"
#include "LunarGlyph.h"
#include "ImGuiMatrix.h"
#include "LunarTime.h"

namespace Fractonica {
    class DesktopApp {
        LunarTime lunarTime;
        UnixClock unixClock;
        ImGuiMatrix matrix;
        LunarGlyph glyph;

        public:
        DesktopApp();
        void run();
        void setup();
        void cleanup();
    };

}


#endif
