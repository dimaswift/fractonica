#include "DS1307UnixClock.h"

#define DS1307_CH_BIT 0x80
#define DS1307_SEC_REG 0x00

namespace Fractonica {

bool DS1307UnixClock::begin() {
    Wire.begin();

    // Check if device is alive
    Wire.beginTransmission(_addr);
    if (Wire.endTransmission() != 0) {
        return false; // Device not found
    }

    // Read the seconds register to check the Clock Halt (CH) bit
    Wire.beginTransmission(_addr);
    Wire.write(DS1307_SEC_REG);
    Wire.endTransmission();

    if (Wire.requestFrom(_addr, (uint8_t)1) == 1) {
        uint8_t sec = Wire.read();

        // If CH bit is set (clock stopped), clear it to start the clock
        if (sec & DS1307_CH_BIT) {
            Wire.beginTransmission(_addr);
            Wire.write(DS1307_SEC_REG);
            Wire.write(sec & 0x7F); // Clear bit 7, keep seconds
            Wire.endTransmission();
        }
        return true;
    }
    return false;
}

uint8_t DS1307UnixClock::bcdToDec(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

uint8_t DS1307UnixClock::decToBcd(uint8_t val) {
    return ((val / 10 * 16) + (val % 10));
}

bool DS1307UnixClock::update() {
    Wire.beginTransmission(_addr);
    Wire.write(DS1307_SEC_REG);
    if (Wire.endTransmission() != 0) return false;

    // Request 7 bytes (Sec, Min, Hour, Day, Date, Month, Year)
    if (Wire.requestFrom(_addr, (uint8_t)7) != 7) return false;

    uint8_t rawSec = Wire.read();
    _seconds = bcdToDec(rawSec & 0x7F);
    _minutes = bcdToDec(Wire.read());

    uint8_t rawHour = Wire.read();

    _hour = bcdToDec(rawHour & 0x3F);

    _day = Wire.read(); // Day of week
    _date = bcdToDec(Wire.read());
    _month = bcdToDec(Wire.read());
    _year = bcdToDec(Wire.read());

    return true;
}

void DS1307UnixClock::setTime(uint8_t hour, uint8_t min, uint8_t sec) {
    Wire.beginTransmission(_addr);
    Wire.write(DS1307_SEC_REG);
    Wire.write(decToBcd(sec));      // 0x00 Seconds (CH=0)
    Wire.write(decToBcd(min));      // 0x01 Minutes
    Wire.write(decToBcd(hour));     // 0x02 Hours (Sets 24h mode implicitly)
    Wire.endTransmission();
}

void DS1307UnixClock::setDate(uint8_t day, uint8_t month, uint16_t year) {
    // DS1307 stores year as 0-99.
    if (year >= 2000) year -= 2000;

    Wire.beginTransmission(_addr);
    Wire.write(0x04); // Start at Date register
    Wire.write(decToBcd(day));
    Wire.write(decToBcd(month));
    Wire.write(decToBcd((uint8_t)year));
    Wire.endTransmission();
}

// ---- Unix Conversions ----

const uint8_t daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

bool DS1307UnixClock::isLeapYear(int year) {
    if (year % 4 != 0) return false;
    if (year % 100 != 0) return true;
    return (year % 400 == 0);
}

uint32_t DS1307UnixClock::now() {
    if (!update()) return 0;

    return unixFromYMDHMS(2000 + _year, _month, _date, _hour, _minutes, _seconds);
}

void DS1307UnixClock::setUnixEpoch(uint32_t t) {
    int year, mon, mday, hour, min, sec;
    ymdhmsFromUnix(t, year, mon, mday, hour, min, sec);

    // Set Time and Date
    setTime(hour, min, sec);
    setDate(mday, mon, year);

    // Note: Day of week is not calculated here, but DS1307 requires it
    // if you use it. You can calculate it from the epoch if needed.
}

uint32_t DS1307UnixClock::unixFromYMDHMS(int year, int mon, int mday,
                                         int hour, int min, int sec) {
    uint32_t days = 0;
    for (int y = 1970; y < year; ++y) {
        days += isLeapYear(y) ? 366UL : 365UL;
    }
    for (int m = 1; m < mon; ++m) {
        days += daysInMonth[m - 1];
        if (m == 2 && isLeapYear(year)) days++;
    }
    days += (mday - 1);

    uint32_t total_seconds = days * 86400UL;
    total_seconds += hour * 3600UL;
    total_seconds += min * 60UL;
    total_seconds += sec;
    return total_seconds;
}

void DS1307UnixClock::ymdhmsFromUnix(uint32_t t,
                                     int &year, int &mon, int &mday,
                                     int &hour, int &min, int &sec) {
    uint32_t days = t / 86400UL;
    uint32_t rem  = t % 86400UL;

    hour = rem / 3600UL;
    rem %= 3600UL;
    min = rem / 60UL;
    sec = rem % 60UL;

    year = 1970;
    while (true) {
        uint16_t daysThisYear = isLeapYear(year) ? 366 : 365;
        if (days >= daysThisYear) {
            days -= daysThisYear;
            year++;
        } else {
            break;
        }
    }

    mon = 1;
    while (true) {
        uint8_t dim = daysInMonth[mon - 1];
        if (mon == 2 && isLeapYear(year)) dim++;
        if (days >= dim) {
            days -= dim;
            mon++;
        } else {
            break;
        }
    }
    mday = days + 1;
}

} // namespace Fractonica