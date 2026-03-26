//
// Created by Dmitry Popov on 13.03.2026.
//

#ifndef FRACTONICA_ITIME_H
#define FRACTONICA_ITIME_H
#include <cstdint>

namespace Fractonica {
    class ITime {
    public:
        virtual ~ITime() = default;
        virtual int64_t millis() = 0;
        void delay(int64_t ms);
    };
}

#endif //FRACTONICA_ITIME_H