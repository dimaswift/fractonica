#include "hw_interface.h"
#include <Arduino.h>

void HardwareInterface::init(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset) {
    resetPin = reset;

    csPort = portOutputRegister(digitalPinToPort(cs));
    cdPort = portOutputRegister(digitalPinToPort(cd));
    wrPort = portOutputRegister(digitalPinToPort(wr));
    rdPort = portOutputRegister(digitalPinToPort(rd));

    csPinMask = digitalPinToBitMask(cs);
    cdPinMask = digitalPinToBitMask(cd);
    wrPinMask = digitalPinToBitMask(wr);
    rdPinMask = digitalPinToBitMask(rd);

    csInvMask = ~csPinMask;
    cdInvMask = ~cdPinMask;
    wrInvMask = ~wrPinMask;
    rdInvMask = ~rdPinMask;

    *csPort |= csPinMask;
    *cdPort |= cdPinMask;
    *wrPort |= wrPinMask;
    *rdPort |= rdPinMask;

    pinMode(cs, OUTPUT);
    pinMode(cd, OUTPUT);
    pinMode(wr, OUTPUT);
    pinMode(rd, OUTPUT);

    if (reset != 255) {
        digitalWrite(reset, HIGH);
        pinMode(reset, OUTPUT);
    }

    SET_WRITE_DIR();
}

void HardwareInterface::reset() {
    setCS(false);
    *rdPort |= rdPinMask;
    *wrPort |= wrPinMask;

    if (resetPin != 255) {
        digitalWrite(resetPin, LOW);
        delay(10);
        digitalWrite(resetPin, HIGH);
        delay(150);
    }

    setCS(true);
    *cdPort &= cdInvMask;
    write8(0x00);
    for (uint8_t i = 0; i < 3; i++) {
        strobeWrite();
    }
    setCS(false);
}
