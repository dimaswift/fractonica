//
// Created by Dmitry Popov on 29.01.2026.
//

#ifndef FRACTONICA_TRANSIENTUNIXCLOCK_H
#define FRACTONICA_TRANSIENTUNIXCLOCK_H
#include "IUnixClock.h"

namespace Fractonica {
    class TransientUnixClock : public IUnixClock {
        int64_t last_ = 0;
        int64_t offset_ = 0;
    public:
        explicit TransientUnixClock(const int64_t offset = 0) {
            offset_ = offset;
            update(0);
        }

        int64_t now() override;

        void update(const uint64_t timeSinceStartUp);

    };

    inline  int64_t TransientUnixClock::now() {
        return last_;
    }

    inline void TransientUnixClock::update(const uint64_t timeSinceStartUp) {
        last_ = timeSinceStartUp + offset_;
    }
}

#endif //FRACTONICA_TRANSIENTUNIXCLOCK_H