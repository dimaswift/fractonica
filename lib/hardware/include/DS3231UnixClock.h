#ifndef DS3231UNIXCLOCK_H
#define DS3231UNIXCLOCK_H

#include <IUnixClock.h>
#include "DS3231.h"
#include <Wire.h>

namespace Fractonica {
    class DS3231UnixClock : public IUnixClock
    {
    private:
        RTClib rtcLib;
        DS3231 rtc;
    public:
        int64_t now() override;
        void setUnix(int32_t now);
    };
}


#endif