#ifndef CSPICE_UTILS_HPP
#define CSPICE_UTILS_HPP

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <cmath>

extern "C" {
#include "../cspice/include/SpiceUsr.h"
}

#if defined(_WIN32)
#include <time.h> // _mkgmtime
#endif

namespace cspice_utils {
    // Convert Ephemeris Time (ET, seconds past J2000 TDB) to UTC string.
    // format: ISOC, precision: 0 (seconds)
    inline void et_to_utc_string(SpiceDouble et, char *utc_str, int max_len) {
        if (!utc_str || max_len <= 0) return;
        et2utc_c(et, "ISOC", 0, (SpiceInt) max_len, utc_str);
    }


    // Convert a local time (ISO-like string understood by SPICE) to UTC string by applying a fixed offset.
    // NOTE: This is a simplistic offset correction; real-world time zones/DST require a proper tz database.
    inline void local_time_to_utc(const char *local_time_str,
                                  double timezone_offset_hours,
                                  char *utc_str,
                                  int max_len) {
        if (!utc_str || max_len <= 0) return;
        if (!local_time_str) {
            std::snprintf(utc_str, (size_t) max_len, "ERROR");
            return;
        }

        SpiceDouble local_et = 0.0;
        str2et_c(local_time_str, &local_et);

        // Convert hours to seconds and subtract to get UTC ET.
        const SpiceDouble utc_et = local_et - (timezone_offset_hours * 3600.0);
        et_to_utc_string(utc_et, utc_str, max_len);
    }

    // Convert Unix time_t to UTC string "YYYY-MM-DDTHH:MM:SS"
    inline void unix_time_to_utc(std::time_t unix_time, char *utc_str, int max_len) {
        if (!utc_str || max_len <= 0) return;

        std::tm *utc_tm = std::gmtime(&unix_time);
        if (!utc_tm) {
            std::snprintf(utc_str, (size_t) max_len, "ERROR");
            return;
        }

        std::strftime(utc_str, (size_t) max_len, "%Y-%m-%dT%H:%M:%S", utc_tm);
    }

    // Convert ET to Unix time_t by formatting as UTC then parsing and converting to epoch seconds.
    // NOTE: This uses ET->UTC formatting; accuracy is limited by the string precision (seconds here).
    inline std::time_t et_to_unix_time(SpiceDouble et) {
        char utc_str[32] = {};
        et2utc_c(et, "ISOC", 0, (SpiceInt) sizeof(utc_str), utc_str);

        std::tm tm_utc{};
        int year = 0, mon = 0, mday = 0, hour = 0, min = 0, sec = 0;

        if (std::sscanf(utc_str, "%4d-%2d-%2dT%2d:%2d:%2d",
                        &year, &mon, &mday, &hour, &min, &sec) != 6) {
            // Keep behavior similar to your original code:
            std::fprintf(stderr, "Failed to parse UTC string '%s'\n", utc_str);
            return (std::time_t) -1;
        }

        tm_utc.tm_year = year - 1900;
        tm_utc.tm_mon = mon - 1;
        tm_utc.tm_mday = mday;
        tm_utc.tm_hour = hour;
        tm_utc.tm_min = min;
        tm_utc.tm_sec = sec;
        tm_utc.tm_isdst = 0;

#if defined(_WIN32)
        return (std::time_t) _mkgmtime(&tm_utc);
#else
        return (std::time_t) timegm(&tm_utc);
#endif
    }

    // Convert UTC string (any SPICE-supported time string) to ET.
    inline SpiceDouble utc_to_et(const char *utc_str) {
        SpiceDouble et = 0.0;
        if (!utc_str) return et;
        str2et_c(utc_str, &et);
        return et;
    }

    // Get current system time as UTC string "YYYY-MM-DDTHH:MM:SS"
    inline void get_current_utc_time(char *utc_str, int max_len) {
        if (!utc_str || max_len <= 0) return;

        std::time_t current_time{};
        std::time(&current_time);

        std::tm *utc_tm = std::gmtime(&current_time);
        if (!utc_tm) {
            std::snprintf(utc_str, (size_t) max_len, "ERROR");
            return;
        }

        std::strftime(utc_str, (size_t) max_len, "%Y-%m-%dT%H:%M:%S", utc_tm);
    }

    // Format time difference (seconds) as human-readable days/hours/minutes.
    inline void format_time_difference(double time_diff_seconds, char *output, int max_len) {
        if (!output || max_len <= 0) return;

        const int sign = (time_diff_seconds >= 0) ? 1 : -1;
        const double abs_diff = std::fabs(time_diff_seconds);

        const int days = (int) (abs_diff / 86400.0);
        const int hours = (int) ((abs_diff - days * 86400.0) / 3600.0);
        const int minutes = (int) ((abs_diff - days * 86400.0 - hours * 3600.0) / 60.0);

        if (days > 0) {
            std::snprintf(output, (size_t) max_len, "%s%d days %d hours %d minutes",
                          (sign < 0) ? "-" : "", days, hours, minutes);
        } else if (hours > 0) {
            std::snprintf(output, (size_t) max_len, "%s%d hours %d minutes",
                          (sign < 0) ? "-" : "", hours, minutes);
        } else {
            std::snprintf(output, (size_t) max_len, "%s%d minutes",
                          (sign < 0) ? "-" : "", minutes);
        }
    }

    // Get current time as ET by converting current UTC string through SPICE.
    inline SpiceDouble get_current_time_et() {
        char time_utc_str[32] = {};
        get_current_utc_time(time_utc_str, (int) sizeof(time_utc_str));
        SpiceDouble et = 0.0;
        str2et_c(time_utc_str, &et);
        return et;
    }

    // --- Optional: JD <-> Unix helpers (UTC-based) ---
    // JD at Unix epoch (1970-01-01T00:00:00Z) is 2440587.5
    inline double jd_to_unix_seconds(double jd_utc) {
        return (jd_utc - 2440587.5) * 86400.0;
    }

    inline std::int64_t jd_to_unix_seconds_i64(double jd_utc) {
        return (std::int64_t) llround(jd_to_unix_seconds(jd_utc));
    }

    inline double unix_seconds_to_jd(double unix_seconds) {
        return (unix_seconds / 86400.0) + 2440587.5;
    }
} // namespace cspice_utils

#endif // CSPICE_UTILS_HPP
