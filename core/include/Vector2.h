#ifndef VECTOR2_H
#define VECTOR2_H
#include <stdint.h>

struct Vector2 {
    int16_t x, y;
    constexpr Vector2() : x(0), y(0) { }
    constexpr Vector2(const int16_t _x, const int16_t _y) : x(_x), y(_y) { }
};

#endif