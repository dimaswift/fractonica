#pragma once
#include <App.h>
#include <SFMLDisplay.h>
#include <UnixClock.h>
#include <LunarClockApp.h>
#include <ImGuiInput.h>
#include <csignal>

inline volatile sig_atomic_t keepRunning = 1;

inline void sigintHandler(int)
{
    keepRunning = 0;
}

int main()
{
    signal(SIGINT, sigintHandler);

    Fractonica::ImGuiInput input;
    Fractonica::IDisplay *display = new Fractonica::SFMLDisplay(320, 480, 1, &input);
    Fractonica::IUnixClock *unixClock = new Fractonica::UnixClock();
    Fractonica::LunarClockApp app(display, unixClock, &input);

    app.setup();

    sf::Clock clock;

    while (keepRunning && app.running())
    {
        const int32_t dt = static_cast<int32_t>(clock.restart().asMicroseconds());
        app.loop(dt);
    }

    delete display;
    delete unixClock;

    return 0;
}
