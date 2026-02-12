//
// Created by Dmitry Popov on 11.02.2026.
//

#ifndef FRACTONICA_GUI_H
#define FRACTONICA_GUI_H

#include <stdint.h>
#include "imgui.h"


namespace Fractonica {

    class Gui {

    public:
        static bool UnixDatePicker(const char* label,
                                       int* year, int* month, int* day,
                                       int64_t* unixOut);
        static void Timestamp(int64_t t, const char *label);
        static void Duration(int64_t seconds);
    };

    static int is_leap(int y) {
        return (y % 4 == 0) && ((y % 100) != 0 || (y % 400) == 0);
    }

    static int days_in_month(int y, int m) { // m: 1..12
        static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (m == 2) return mdays[1] + (is_leap(y) ? 1 : 0);
        return mdays[m - 1];
    }

    inline void Gui::Duration(int64_t seconds) {
        size_t out_sz = 128;
        char out[128];
        int neg = (seconds < 0);
        uint64_t s = (seconds < 0) ? static_cast<uint64_t>(-(seconds + 1)) + 1u : static_cast<uint64_t>(seconds);

        constexpr uint64_t SEC_MIN  = 60;
        constexpr uint64_t SEC_HOUR = 60 * SEC_MIN;
        constexpr uint64_t SEC_DAY  = 24 * SEC_HOUR;
        constexpr uint64_t SEC_YEAR = 365 * SEC_DAY;

        uint64_t years = s / SEC_YEAR; s %= SEC_YEAR;
        uint64_t days  = s / SEC_DAY;  s %= SEC_DAY;
        uint64_t hours = s / SEC_HOUR; s %= SEC_HOUR;
        uint64_t mins  = s / SEC_MIN;  s %= SEC_MIN;
        uint64_t secs  = s;

        size_t n = 0;
        int wrote_any = 0;

        if (neg) n += static_cast<size_t>(snprintf(out + n, (n < out_sz) ? out_sz - n : 0, "-"));

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
            n += static_cast<size_t>(snprintf(out + n, (n < out_sz) ? out_sz - n : 0, "0 secs"));
        }

        ImGui::TextUnformatted(out);

    }

    static int64_t days_from_civil(int y, unsigned m, unsigned d) {
        y -= (m <= 2);
        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = (unsigned)(y - era * 400);                  // [0, 399]
        const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;      // [0, 146096]
        return (int64_t)era * 146097 + (int64_t)doe - 719468;
    }

    static int64_t unix_from_ymd_utc(int y, int m, int d) {
        int64_t days = days_from_civil(y, (unsigned)m, (unsigned)d);
        return days * 86400LL; // 00:00:00 UTC
    }

    static const char* getMonthName(const int month) {
        switch (month) {
            case 1: return "Jan";
            case 2: return "Feb";
            case 3: return "Mar";
            case 4: return "Apr";
            case 5: return "May";
            case 6: return "Jun";
            case 7: return "Jul";
            case 8: return "Aug";
            case 9: return "Sep";
            case 10: return "Oct";
            case 11: return "Nov";
            case 12: return "Dec";
            default:
                return "Invalid";
        }
    }

    inline void Gui::Timestamp(const int64_t t, const char *label) {
        const DateTimeUTC dt = Utils::UnixToUTC(t);
        ImGui::Text("%s: %02d:%02d:%02d %d %s %04d", label, dt.hour, dt.minute, dt.second, dt.day, getMonthName(dt.month), dt.year);
    }

    inline  bool Gui::UnixDatePicker(const char* label,
                                   int* year, int* month, int* day,
                                   int64_t* unixOut)
    {
        bool changed = false;

        ImGui::PushID(label);
        ImGui::TextUnformatted(label);

        // Year input
        int y = *year;
        changed |= ImGui::InputInt("Year", &y);
        if (y < -9999) y = -9999;
        if (y >  9999) y =  9999;

        // Month slider
        int mo = *month;
        if (mo < 1) mo = 1;
        if (mo > 12) mo = 12;
        changed |= ImGui::SliderInt("Month", &mo, 1, 12);

        // Day slider (range depends on year+month)
        int maxDay = days_in_month(y, mo);
        int da = *day;
        if (da < 1) da = 1;
        if (da > maxDay) da = maxDay;
        changed |= ImGui::SliderInt("Day", &da, 1, maxDay);

        // Commit back
        *year = y; *month = mo; *day = da;

        // Compute unix timestamp (UTC midnight)
        if (unixOut) *unixOut = unix_from_ymd_utc(y, mo, da);

        ImGui::PopID();
        return changed;
    }
}


#endif //FRACTONICA_GUI_H