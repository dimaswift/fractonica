#ifndef IINPUT_H
#define IINPUT_H
#include <stdint.h>

namespace Fractonica
{
    enum class InputEventType
    {
        BUTTON_PRESSED,
        BUTTON_RELEASED,
        VALUE_CHANGED,
    };

    struct InputEvent
    {
        InputEventType type;
        uint8_t id;
        int32_t value;
        const char *name;
    };

    typedef void (*InputCallback)(const InputEvent &event, void *context);

    class IInput
    {
    public:
        virtual ~IInput() = default;
        virtual void update() = 0;
        virtual void setCallback(InputCallback callback, void *context) = 0;
    };

} // namespace Fractonica

#endif