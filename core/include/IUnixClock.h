#ifndef IUNIXCLOCK_H
#define IUNIXCLOCK_H
#include <stdint.h>

namespace Fractonica {
    class IUnixClock
    {

    public:
        virtual ~IUnixClock() = default;
        virtual int64_t now() = 0;
    };
}

#endif