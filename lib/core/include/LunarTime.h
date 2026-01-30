//
// Created by Dmitry Popov on 29.01.2026.
//

#ifndef FRACTONICA_LUNARTIME_H
#define FRACTONICA_LUNARTIME_H
#include <stdint.h>
#include "Ephemeris.h"

namespace Fractonica {

    enum LunarEvent {
        NEW_MOON,
        APOGEE,
        NODAL_ASCENDING
    };

    struct LunarEventInfo {
        uint32_t bin;
        uint32_t binOctal;
        double progress;
        LunarEvent event;
        double normalized;
    };

    class LunarTime {

        fractonica_mem_source_t newMoon{}, apogee{}, nodalAscending{};
        uint32_t prevNewMoon = 0;
        uint32_t prevApogee = 0;
        uint32_t prevNodalAscending = 0;
        uint32_t resolution;
        uint8_t digits;
        uint8_t base;
    public:
        explicit LunarTime(uint8_t digits = 4, uint8_t base = 8);
        [[nodiscard]] LunarEventInfo getEventInfo(uint32_t timestamp,  LunarEvent type) const;

    };
}

#endif //FRACTONICA_LUNARTIME_H