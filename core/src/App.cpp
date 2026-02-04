#include <App.h>

namespace Fractonica
{
    App::App(IDisplay *display, IUnixClock *unixClock, IInput *input) : display_(display), unixClock_(unixClock), input_(input)
    {
        if (input_)
        {
            input_->setCallback(&App::staticInputHandler, this);
        }
    }

    void App::staticInputHandler(const InputEvent &event, void *context)
    {
        const auto app = static_cast<App *>(context);
        app->onInput(event);
    }

    void App::loop(int32_t dt) 
    {
        if (input_)
        {
            input_->update();
        }
    }
    
    bool App::running() 
    {
        return display_->isOpen();
    }

    void App::setup()
    {
        display_->begin();
    }
}