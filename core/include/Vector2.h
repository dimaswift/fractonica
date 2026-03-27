#ifndef VECTOR2_H
#define VECTOR2_H
#include <stdint.h>

struct Vector2 {
    int16_t x, y;

    constexpr Vector2() : x(0), y(0) { }
    constexpr Vector2(const int16_t _x, const int16_t _y) : x(_x), y(_y) { }

    constexpr Vector2 operator+(const Vector2& rhs) const {
        return Vector2(x + rhs.x, y + rhs.y);
    }
    constexpr Vector2& operator+=(const Vector2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr Vector2 operator-(const Vector2& rhs) const {
        return Vector2(x - rhs.x, y - rhs.y);
    }
    constexpr Vector2& operator-=(const Vector2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr Vector2 operator*(const int16_t scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    constexpr Vector2& operator*=(const int16_t scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2 operator/(const int16_t scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    constexpr Vector2& operator/=(const int16_t scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // --- Unary Minus (Negation) ---
    constexpr Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    // --- Equality ---
    constexpr bool operator==(const Vector2& rhs) const {
        return x == rhs.x && y == rhs.y;
    }
    constexpr bool operator!=(const Vector2& rhs) const {
        return x != rhs.x || y != rhs.y;
    }
};

#endif