#ifndef HW_INTERFACE_H
#define HW_INTERFACE_H

#include <stdint.h>
#include <avr/io.h>

// 16-bit parallel interface using PORTA (high byte) and PORTC (low byte)
#define WRITE_DATA8_NOSTROBE(d) { PORTC = (d); }
#define WRITE_DATA16_NOSTROBE(d) { PORTA = ((d) >> 8); PORTC = (d); }
#define READ_DATA16() ((PINA << 8) | PINC)
#define SET_WRITE_DIR() { DDRC = 0xFF; DDRA = 0xFF; }
#define SET_READ_DIR() { DDRC = 0x00; DDRA = 0x00; }

class HardwareInterface {
private:
    volatile uint8_t *csPort, *cdPort, *wrPort, *rdPort;
    uint8_t csPinMask, cdPinMask, wrPinMask, rdPinMask;
    uint8_t csInvMask, cdInvMask, wrInvMask, rdInvMask;
    uint8_t resetPin;

    inline void strobeWrite() __attribute__((always_inline)) {
        *wrPort &= wrInvMask;
        *wrPort |= wrPinMask;
    }

    inline void strobeRead() __attribute__((always_inline)) {
        *rdPort |= rdPinMask;
        *rdPort &= rdInvMask;
        *rdPort &= rdInvMask;
        *rdPort &= rdInvMask;
    }

public:
    void init(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset);
    void reset();

    inline void setCS(bool active) __attribute__((always_inline)) {
        if (active) *csPort &= csInvMask;
        else *csPort |= csPinMask;
    }

    inline void setCD(bool data) __attribute__((always_inline)) {
        if (data) *cdPort |= cdPinMask;
        else *cdPort &= cdInvMask;
    }

    inline void write8(uint8_t data) __attribute__((always_inline)) {
        WRITE_DATA8_NOSTROBE(data);
        strobeWrite();
    }

    inline void write16(uint16_t data) __attribute__((always_inline)) {
        WRITE_DATA16_NOSTROBE(data);
        strobeWrite();
    }

    inline uint8_t read8() __attribute__((always_inline)) {
        *rdPort |= rdPinMask;
        *rdPort &= rdInvMask;
        *rdPort &= rdInvMask;
        *rdPort &= rdInvMask;
        uint16_t result = READ_DATA16();
        *rdPort |= rdPinMask;
        return result & 0xFF;
    }

    inline uint16_t read16() __attribute__((always_inline)) {
        *rdPort |= rdPinMask;
        *rdPort &= rdInvMask;
        *rdPort &= rdInvMask;
        *rdPort &= rdInvMask;
        uint16_t result = READ_DATA16();
        *rdPort |= rdPinMask;
        return result;
    }

    inline void writeCmd(uint8_t cmd) __attribute__((always_inline)) {
        *cdPort &= cdInvMask;
        WRITE_DATA8_NOSTROBE(cmd);
        strobeWrite();
        *cdPort |= cdPinMask;
    }

    inline void writeData(uint8_t data) __attribute__((always_inline)) {
        WRITE_DATA8_NOSTROBE(data);
        strobeWrite();
    }

    inline void writeCmd16(uint16_t cmd) __attribute__((always_inline)) {
        setCD(false);
        write16(cmd);
        setCD(true);
    }

    inline void writeData16(uint16_t data) __attribute__((always_inline)) {
        write16(data);
    }

    inline void writeCmdData8(uint8_t cmd, uint8_t data) __attribute__((always_inline)) {
        writeCmd(cmd);
        writeData(data);
    }

    inline void writeCmdData16(uint16_t cmd, uint16_t data) __attribute__((always_inline)) {
        writeCmd16(cmd);
        writeData16(data);
    }
};

#endif
