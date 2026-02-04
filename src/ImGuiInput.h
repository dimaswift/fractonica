// ImGuiInput.h
#ifndef IMGUI_INPUT_H
#define IMGUI_INPUT_H

#include "IInput.h"
#include <imgui.h>
#include <unordered_map>
#include <string>

namespace Fractonica
{
    class ImGuiInput : public IInput
    {
    private:
        void *context_ = nullptr;
        InputCallback callback_ = nullptr;
        std::unordered_map<std::string, int32_t> values_;

    public:
        void update() override
        {

        }

        void setCallback(InputCallback callback, void *context) override
        {
            callback_ = callback;
            context_ = context;
        }

        void slider(const char *label, int32_t *value, const int32_t min, const int32_t max, const uint8_t id) const {
            const int32_t oldValue = *value;
            ImGui::SliderInt(label, value, min, max);

            if (*value != oldValue && callback_)
            {
                callback_({InputEventType::VALUE_CHANGED, id, *value, label}, context_);
            }
        }

        bool button(const char *label, const uint8_t id) const {
            if (ImGui::Button(label))
            {
                if (callback_)
                {
                    callback_({InputEventType::BUTTON_PRESSED, id, 0, label}, context_);
                }
                return true;
            }
            return false;
        }
    };

} // namespace Fractonica::Desktop

#endif