#include "UnixClock.h"

namespace Fractonica
{
    int64_t UnixClock::now()
    {
        return std::time(nullptr);
    }
}