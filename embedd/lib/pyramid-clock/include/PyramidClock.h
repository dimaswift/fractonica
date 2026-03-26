#include "Arduino.h"
#include "../../esp32/include/WifiClock.h"
#include "saros.h"
#include "OctalDigitFace.h"

Fractonica::OctalDigitFace face(5, 6, 7);
Fractonica::WifiClock wifiClock("RT-GPON-7", "857010486557");

uint64_t lastSync = 0;
int sarosNumber = 141;

static const uint8_t DIGITS[4][8] = {
    //   0           1           2            3           4           5          6             7
    {0b00000000, 0b00000001, 0b01010000, 0b00001001, 0b00000010, 0b10100001, 0b01010100, 0b00001101}, // 0
    {0b00000000, 0b00000010, 0b10100000, 0b00000011, 0b00000100, 0b01010010, 0b10101000, 0b00001011}, // 1
    {0b00000000, 0b00000100, 0b01010000, 0b00000110, 0b00001000, 0b10100100, 0b01010001, 0b00000111}, // 2
    {0b00000000, 0b00001000, 0b10100000, 0b00001100, 0b00000001, 0b01011000, 0b10100010, 0b00001110}  // 3
};

void setup()
{
    face.begin();
    wifiClock.begin();
    wifiClock.update();
}

void loop()
{
    if (millis() - lastSync > 60000)
    {
        wifiClock.update();
        lastSync = millis();
    }
    int64_t phase = calculate_solar_octal_phase(wifiClock.now(), sarosNumber, 2);
    face.setValue(phase, DIGITS);
    delay(1000);
}