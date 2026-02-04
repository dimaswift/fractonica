#ifndef DS1307_UNIX_CLOCK_H
#define DS1307_UNIX_CLOCK_H

#include <Arduino.h>
#include <Wire.h>

namespace Fractonica {

    class DS1307UnixClock {
    public:
        DS1307UnixClock(uint8_t addr = 0x68) : _addr(addr) {}

        // Returns true if device is connected and running
        bool begin();

        // Updates internal variables from the chip. Returns true on success.
        bool update();

        // Unix Timestamp methods
        uint32_t now();
        void setUnixEpoch(uint32_t t);

        // Getters
        uint8_t getSeconds() const { return _seconds; }
        uint8_t getMinute() const { return _minutes; }
        uint8_t getHour() const { return _hour; }
        uint8_t getDay() const { return _day; }     // Day of week (1-7)
        uint8_t getDate() const { return _date; }   // Day of month (1-31)
        uint8_t getMonth() const { return _month; }
        uint8_t getYear() const { return _year; }   // 2-digit year (00-99)

        // Setters (integers are safer/faster than strings)
        void setTime(uint8_t hour, uint8_t min, uint8_t sec);
        void setDate(uint8_t day, uint8_t month, uint16_t year); // year can be 2024 or 24

    private:
        uint8_t _addr;
        uint8_t _seconds, _minutes, _hour;
        uint8_t _day, _date, _month, _year;

        // Helpers
        uint8_t bcdToDec(uint8_t val);
        uint8_t decToBcd(uint8_t val);

        // Unix Helpers
        bool isLeapYear(int year);
        uint32_t unixFromYMDHMS(int year, int mon, int mday, int hour, int min, int sec);
        void ymdhmsFromUnix(uint32_t t, int &year, int &mon, int &mday, int &hour, int &min, int &sec);
    };

} // namespace Fractonica

#endif