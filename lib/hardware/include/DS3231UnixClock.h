#ifndef DS3231UNIXCLOCK_H
#define DS3231UNIXCLOCK_H

#include <IUnixClock.h>
#include "DS3231.h"
#include <Wire.h>

namespace Fractonica {
    class DS3231UnixClock : public IUnixClock
    {
    private:
        RTClib myRTC;

    public:
        int64_t now() override;
    };
}


#endif