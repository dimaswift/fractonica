#pragma once
#include <SFMLDisplay.h>
#include <UnixClock.h>
#include <LunarGlyph.h>
#include <ImGuiInput.h>
#include <csignal>
#include "ImGuiMatrix.h"
#include "LunarTime.h"

inline volatile sig_atomic_t keepRunning = 1;
static Fractonica::LunarTime lunarTime;
static Fractonica::UnixClock unixClock;
static Fractonica::ImGuiMatrix matrix(8, 8, 32);
static Fractonica::LunarGlyph glyph;

static int64_t prevSec = 0;

inline void sigintHandler(int)
{
    keepRunning = 0;
}

static void onGui() {

    ImGui::Begin("New Moon");

    Fractonica::LunarEventInfo info = lunarTime.getEventInfo(unixClock.now(), Fractonica::NEW_MOON);

    ImGui::Text("Bin: %d", info.binOctal);
    ImGui::Text("Normalized: %f", info.normalized);
    ImGui::Text("Progress: %f", info.progress);

    glyph.draw(unixClock.now(), &matrix);

    ImGui::End();
}

int main()
{
    signal(SIGINT, sigintHandler);

    Fractonica::ImGuiInput input;
    Fractonica::SFMLDisplay display (1024, 1024, 1, &input);

    display.begin();

    matrix.begin();

    Fractonica::SFMLDisplay::addGuiCallback(onGui);

    while (keepRunning && display.isOpen())
    {
        display.update();
    }

    return 0;
}
