#pragma once

#include <cmath>
#include <concepts>
#include <stdexcept>
#include <algorithm>


template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
class Vector2 
{
public:
    T x{ 0 };
    T y{ 0 };

    constexpr Vector2() noexcept = default;
    constexpr Vector2(T x, T y) noexcept : x(x), y(y) {}

    constexpr Vector2(const Vector2&) noexcept = default;
    constexpr Vector2& operator=(const Vector2&) noexcept = default;
    constexpr Vector2(Vector2&&) noexcept = default;
    constexpr Vector2& operator=(Vector2&&) noexcept = default;

    [[nodiscard]] constexpr T LengthSquared() const noexcept 
    {
        return x * x + y * y;
    }

    [[nodiscard]] T Length() const noexcept 
    {
        return static_cast<T>(std::sqrt(LengthSquared()));
    }

    Vector2& Normalize() noexcept 
    {
        T len = Length();
        if (len != 0) {
            x /= len;
            y /= len;
        }
        return *this;
    }

    [[nodiscard]] Vector2 Normalized() const noexcept 
    {
        Vector2 result = *this;
        result.Normalize();
        return result;
    }

    [[nodiscard]] constexpr T Dot(const Vector2& other) const noexcept 
    {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] constexpr T Cross(const Vector2& other) const noexcept 
    {
        return x * other.y - y * other.x;
    }

    [[nodiscard]] T DistanceTo(const Vector2& other) const noexcept 
    {
        return (*this - other).Length();
    }

    [[nodiscard]] constexpr T DistanceSquaredTo(const Vector2& other) const noexcept 
    {
        return (*this - other).LengthSquared();
    }

    void Rotate(T angle) noexcept 
    {
        T cs = std::cos(angle);
        T sn = std::sin(angle);
        T px = x * cs - y * sn;
        T py = x * sn + y * cs;
        x = px;
        y = py;
    }

    [[nodiscard]] Vector2 Rotated(T angle) const noexcept 
    {
        Vector2 result = *this;
        result.Rotate(angle);
        return result;
    }

    [[nodiscard]] static Vector2 Zero() noexcept { return Vector2(); }
    [[nodiscard]] static Vector2 One() noexcept { return Vector2(1, 1); }
    [[nodiscard]] static Vector2 Up() noexcept { return Vector2(0, -1); }
    [[nodiscard]] static Vector2 Down() noexcept { return Vector2(0, 1); }
    [[nodiscard]] static Vector2 Left() noexcept { return Vector2(-1, 0); }
    [[nodiscard]] static Vector2 Right() noexcept { return Vector2(1, 0); }

    [[nodiscard]] static Vector2 Lerp(const Vector2& a, const Vector2& b, T t) noexcept 
    {
        return a + (b - a) * std::clamp(t, T(0), T(1));
    }

    constexpr Vector2 operator+(const Vector2& other) const noexcept 
    {
        return Vector2(x + other.x, y + other.y);
    }

    constexpr Vector2 operator-(const Vector2& other) const noexcept 
    {
        return Vector2(x - other.x, y - other.y);
    }

    constexpr Vector2 operator*(T scalar) const noexcept 
    {
        return Vector2(x * scalar, y * scalar);
    }

    constexpr Vector2 operator/(T scalar) const 
    {
        if (scalar == 0) throw std::runtime_error("Division by zero");
        return Vector2(x / scalar, y / scalar);
    }

    constexpr Vector2& operator+=(const Vector2& other) noexcept 
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2& operator-=(const Vector2& other) noexcept 
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vector2& operator*=(T scalar) noexcept 
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2& operator/=(T scalar) {
        if (scalar == 0) throw std::runtime_error("Division by zero");
        x /= scalar;
        y /= scalar;
        return *this;
    }

    constexpr bool operator==(const Vector2& other) const noexcept 
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Vector2& other) const noexcept 
    {
        return !(*this == other);
    }
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int>;

template<Numeric T>
constexpr Vector2<T> operator*(T scalar, const Vector2<T>& vector) noexcept 
{
    return vector * scalar;
}