#include "UnixClock.h"
#include <ctime>
namespace Fractonica
{
    int64_t UnixClock::now()
    {
        return std::time(nullptr);
    }
}