// ImGuiInput.h
#ifndef IMGUI_INPUT_H
#define IMGUI_INPUT_H

#include "IInput.h"

namespace Fractonica
{
    class ArduinoInput : public IInput
    {
        InputCallback callback_ = nullptr;

    public:
        void update() override
        {
        }

        void setCallback(const InputCallback callback, void *context) override
        {
            callback_ = callback;
        }
    };

}

#endif