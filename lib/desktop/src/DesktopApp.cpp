//
// Created by Dmitry Popov on 30.01.2026.
//

#include "DesktopApp.h"

namespace Fractonica {

    void DesktopApp::setup() {
        matrix.begin();
    }

    void DesktopApp::run() {
        ImGui::Begin("New Moon");

        LunarEventInfo info = lunarTime.getEventInfo(unixClock.now(), Fractonica::NEW_MOON);

        ImGui::Text("Bin: %d", info.binOctal);
        ImGui::Text("Normalized: %f", info.normalized);
        ImGui::Text("Progress: %f", info.progress);



        ImGui::End();

        glyph.draw(unixClock.now(), &matrix);
    }


    DesktopApp::DesktopApp() : matrix(8, 8, 32) {

    }

    void DesktopApp::cleanup() {

    }

}
