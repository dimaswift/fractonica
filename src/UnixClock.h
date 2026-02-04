#ifndef UNIXCLOCK_H
#define UNIXCLOCK_H
#include "IUnixClock.h"
#include <iostream>
namespace Fractonica
{
    class UnixClock : public IUnixClock
    {
    public :
        UnixClock() = default;
        int64_t now() override;
    };
}

#endif