#include "DS3231UnixClock.h"

#include <stdint.h>

namespace Fractonica {

    int64_t DS3231UnixClock::now() {
        DateTime date = myRTC.now();
        return date.unixtime();
    }
}
