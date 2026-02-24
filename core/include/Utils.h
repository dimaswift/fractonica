//
// Created by Dmitry Popov on 30.01.2026.
//

#ifndef FRACTONICA_UTILS_H
#define FRACTONICA_UTILS_H
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
namespace Fractonica {


    struct DateTimeUTC {
        int year, month, day, hour, minute, second;
    };

    class Utils {
    public:
        static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
        static uint32_t ColorHSV(uint16_t hue, uint8_t sat, uint8_t val);
        static DateTimeUTC UnixToUTC(int64_t unix_seconds);
        static size_t DurationToText(int64_t seconds, char *out, size_t out_sz);
        static const char* FormatFrequency(int64_t freq_hz, char *buffer, size_t buffer_size);
        static int64_t DecimaToOctal(int64_t decimalNumber);
    };

    inline int64_t Utils::DecimaToOctal(int64_t decimalNumber) {
        int64_t octalNumber = 0;
        int64_t i = 1;
        int64_t tempDecimal = decimalNumber;
        while (tempDecimal != 0)
        {
            int64_t remainder = tempDecimal % 8;
            octalNumber += remainder * i;
            tempDecimal /= 8;
            i *= 10;
        }
        return octalNumber;
    }

    inline const char* Utils::FormatFrequency(int64_t freq_hz, char *buffer, size_t buffer_size) {

    if (buffer == nullptr || buffer_size < 32) {
        return nullptr;
    }

    int64_t abs_freq = freq_hz < 0 ? -freq_hz : freq_hz;
    const char *sign = freq_hz < 0 ? "-" : "";

    static const struct {
        int64_t threshold;
        const char *suffix;
        double divisor;
    } units[] = {
        { 1000000000000000LL, "PHz", 1e15 },  // 10^15 (petahertz)
        { 1000000000000LL,    "THz", 1e12 },  // 10^12 (terahertz)
        { 1000000000LL,       "GHz", 1e9  },  // 10^9  (gigahertz)
        { 1000000LL,          "MHz", 1e6  },  // 10^6  (megahertz)
        { 1000LL,             "kHz", 1e3  },  // 10^3  (kilohertz)
        { 0,                  "Hz",  1.0  },  // Base unit
    };

    for (size_t i = 0; i < sizeof(units) / sizeof(units[0]); i++) {
        if (abs_freq >= units[i].threshold) {
            double value = (double)freq_hz / units[i].divisor;

            if (abs_freq < units[i].threshold * 10) {

                snprintf(buffer, buffer_size, "%s%.3f %s", sign, fabs(value), units[i].suffix);
            } else if (abs_freq < units[i].threshold * 100) {

                snprintf(buffer, buffer_size, "%s%.2f %s", sign, fabs(value), units[i].suffix);
            } else if (abs_freq < units[i].threshold * 1000) {

                snprintf(buffer, buffer_size, "%s%.1f %s", sign, fabs(value), units[i].suffix);
            } else {

                snprintf(buffer, buffer_size, "%s%.0f %s", sign, fabs(value), units[i].suffix);
            }

            char *dot = strchr(buffer, '.');
            if (dot) {
                char *end = buffer + strlen(buffer) - 1;
                while (end > dot && *end != ' ') end--;
                char *space = end;
                end--;

                while (end > dot && *end == '0') {
                    end--;
                }

                if (end == dot) {
                    end--;
                }

                end++;
                memmove(end, space, strlen(space) + 1);
            }

            return buffer;
        }
    }

    snprintf(buffer, buffer_size, "%s%lld Hz", sign, (long long)freq_hz);
    return buffer;
}

    inline uint32_t Utils::Color(uint8_t r, uint8_t g, uint8_t b) {
        return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }

    inline size_t Utils::DurationToText(int64_t seconds, char *out, size_t out_sz) {
        if (!out || out_sz == 0) return 0;

        // Handle sign
        int neg = (seconds < 0);
        uint64_t s = (seconds < 0) ? (uint64_t)(-(seconds + 1)) + 1u : (uint64_t)seconds;

        const uint64_t SEC_MIN  = 60;
        const uint64_t SEC_HOUR = 60 * SEC_MIN;
        const uint64_t SEC_DAY  = 24 * SEC_HOUR;
        const uint64_t SEC_YEAR = 365 * SEC_DAY; // "year" = 365 days

        uint64_t years = s / SEC_YEAR; s %= SEC_YEAR;
        uint64_t days  = s / SEC_DAY;  s %= SEC_DAY;
        uint64_t hours = s / SEC_HOUR; s %= SEC_HOUR;
        uint64_t mins  = s / SEC_MIN;  s %= SEC_MIN;
        uint64_t secs  = s;

        size_t n = 0;
        int wrote_any = 0;

        if (neg) n += (size_t)snprintf(out + n, (n < out_sz) ? out_sz - n : 0, "-");

        // helper to append "N unit(s)" with optional leading space
        #define APPEND_PART(val, singular, plural) do {                           \
            if ((val) != 0) {                                                     \
                n += (size_t)snprintf(out + n, (n < out_sz) ? out_sz - n : 0,     \
                                      "%s%llu %s",                                \
                                      wrote_any ? " " : "",                       \
                                      (unsigned long long)(val),                  \
                                      ((val) == 1) ? (singular) : (plural));      \
                wrote_any = 1;                                                    \
            }                                                                     \
        } while (0)

        APPEND_PART(years, "year", "years");
        APPEND_PART(days,  "day",  "days");
        APPEND_PART(hours, "hour", "hours");
        APPEND_PART(mins,  "min",  "mins");
        APPEND_PART(secs,  "sec",  "secs");

        #undef APPEND_PART

        if (!wrote_any) {
            n += (size_t)snprintf(out + n, (n < out_sz) ? out_sz - n : 0, "0 secs");
        }

        // Ensure NUL-termination even if truncated
        if (out_sz) out[out_sz - 1] = '\0';
        return n; // number of chars that would have been written (like snprintf)
    }

    static int64_t floor_div(int64_t a, int64_t b) {
        // floor(a/b) for b>0
        int64_t q = a / b;
        int64_t r = a % b;
        if (r && ((r > 0) != (b > 0))) --q;
        return q;
    }

    static int64_t floor_mod(int64_t a, int64_t b) {
        int64_t r = a % b;
        if (r < 0) r += b;
        return r;
    }

    // Convert "days since 1970-01-01" to civil date (UTC)
    // Algorithm: Howard Hinnant's civil_from_days (integer, fast)
    static  void civil_from_days(int64_t z, int &y, int &m, int &d) {
        z += 719468; // shift to days since 0000-03-01
        int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        uint32_t doe = (uint32_t)(z - era * 146097);                 // [0, 146096]
        uint32_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365; // [0, 399]
        int64_t y_ = (int64_t)yoe + era * 400;
        uint32_t doy = doe - (365*yoe + yoe/4 - yoe/100);            // [0, 365]
        uint32_t mp  = (5*doy + 2)/153;                              // [0, 11]
        uint32_t d_  = doy - (153*mp + 2)/5 + 1;                     // [1, 31]
        uint32_t m_  = mp + (mp < 10 ? 3 : (uint32_t)-9);            // [1, 12]
        y_ += (m_ <= 2);
        y = (int)y_;
        m = (int)m_;
        d = (int)d_;
    }

    inline DateTimeUTC Utils::UnixToUTC(const int64_t unix_seconds) {
        DateTimeUTC out{};
        const int64_t days = floor_div(unix_seconds, 86400);
        const int64_t sod  = floor_mod(unix_seconds, 86400); // seconds of day [0..86399]
        out.hour   = static_cast<int>(sod / 3600);
        out.minute = static_cast<int>((sod % 3600) / 60);
        out.second = static_cast<int>(sod % 60);
        civil_from_days(days, out.year, out.month, out.day);
        return out;
    }

    inline uint32_t Utils::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {
        uint8_t r, g, b;

        hue = (hue * 1530L + 32768) / 65536;

        if (hue < 510) {
            b = 0;
            if (hue < 255) {
                r = 255;
                g = hue;       //     g = 0 to 254
            } else {         //   Yellow to Green-1
                r = 510 - hue; //     r = 255 to 1
                g = 255;
            }
        } else if (hue < 1020) { // Green to Blue-1
            r = 0;
            if (hue < 765) { //   Green to Cyan-1
                g = 255;
                b = hue - 510;  //     b = 0 to 254
            } else {          //   Cyan to Blue-1
                g = 1020 - hue; //     g = 255 to 1
                b = 255;
            }
        } else if (hue < 1530) { // Blue to Red-1
            g = 0;
            if (hue < 1275) { //   Blue to Magenta-1
                r = hue - 1020; //     r = 0 to 254
                b = 255;
            } else { //   Magenta to Red-1
                r = 255;
                b = 1530 - hue; //     b = 255 to 1
            }
        } else { // Last 0.5 Red (quicker than % operator)
            r = 255;
            g = b = 0;
        }

        const uint32_t v1 = 1 + val;
        const uint16_t s1 = 1 + sat;
        const uint8_t s2 = 255 - sat;
        return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
             (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
             (((((b * s1) >> 8) + s2) * v1) >> 8);
    }

}

#endif //FRACTONICA_UTILS_H