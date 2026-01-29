#ifndef APP_H
#define APP_H

#include "IDisplay.h"
#include "IUnixClock.h"
#include "IInput.h"

namespace Fractonica
{
    class App
    {
        IDisplay *display_;
        IUnixClock *unixClock_;
        IInput *input_;
      

    public:
        App(IDisplay *display, IUnixClock *unixClock, IInput *input);
        virtual ~App() = default;

        virtual void setup();
        virtual void loop(int32_t dt);
        virtual bool running();
        virtual void onInput(const InputEvent &event) {}

        IDisplay *getDisplay() const { return display_; }
        IUnixClock *getUnixClock() const { return unixClock_; }
        IInput *getInput() const { return input_; }
        static void staticInputHandler(const InputEvent &event, void *context);
        void setCallback(InputCallback callback, void *context);
    };
}

#endif