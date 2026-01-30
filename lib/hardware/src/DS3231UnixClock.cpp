#include "DS3231UnixClock.h"
#include "DS3231.h"
#include <stdint.h>

namespace Fractonica {

    int64_t DS3231UnixClock::now() {
        DateTime date = RTClib::now();
        return date.unixtime();
    }

    void DS3231UnixClock::setUnix(int32_t now) {
        rtc.setEpoch(now);
    }
}
