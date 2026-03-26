#ifndef DIGITFACE_74HC595
#define DIGITFACE_74HC595

#include "Arduino.h"
namespace Fractonica
{

    class OctalDigitFace
    {
        int ds, stcp, shcp;

    public:
        OctalDigitFace(int dsPin, int stcpPin, int shcpPin) : ds(dsPin), stcp(stcpPin), shcp(shcpPin)
        {
        }

        void begin()
        {
            pinMode(ds, OUTPUT);
            pinMode(shcp, OUTPUT);
            pinMode(shcp, OUTPUT);
            shiftOut(ds, shcp, MSBFIRST, 0);
        }

        void setValue(uint64_t value, const uint8_t (&digits)[4][8])
        {
            digitalWrite(stcp, LOW);

            for (size_t i = 0; i < 4; i++)
            {
                shiftOut(ds, shcp, MSBFIRST, digits[i][(value >> (3 * i)) % 8]);
            }

            digitalWrite(stcp, HIGH);
        }
    };
}

#endif