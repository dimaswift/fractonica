//
// Created by Dmitry Popov on 29.01.2026.
//

#include "LunarTime.h"

#include "apogee.h"
#include "new_moon.h"
#include "nodal_ascending.h"

namespace Fractonica {
    LunarTime::LunarTime(const uint8_t digits, const uint8_t base) : digits(digits), base(base) {
        fractonica_mem_init(&newMoon, FRACTONICA_NEW_MOON_COUNT, fractonica_new_moon_timestamps);
        fractonica_mem_init(&apogee, FRACTONICA_APOGEE_COUNT, fractonica_apogee_timestamps);
        fractonica_mem_init(&nodalAscending, FRACTONICA_NODAL_ASCENDING_COUNT, fractonica_nodal_ascending_timestamps);
        resolution = static_cast<uint32_t>(pow(base, digits));
    }

    LunarEventInfo LunarTime::getEventInfo(const uint32_t timestamp, const LunarEvent type) const {
        fractonica_ephemeris_fraction_t fraction;
        switch (type) {
            case NEW_MOON:
                fraction = fractonica_ephemeris_fraction_at(&newMoon, timestamp, resolution, nullptr, nullptr);
                break;
            case APOGEE:
                fraction = fractonica_ephemeris_fraction_at(&apogee, timestamp, resolution, nullptr, nullptr);
                break;
            case NODAL_ASCENDING:
                fraction = fractonica_ephemeris_fraction_at(&nodalAscending, timestamp, resolution, nullptr, nullptr);
                break;
            default:
                return LunarEventInfo {};
        }
        return { .bin = fraction.bin, .binOctal = fraction.bin_octal, .progress = fraction.progress, .event = type, .normalized = fraction.normalized  };
    }
}


