#pragma once

#include <SDL3/SDL.h>
#include <concepts>
#include <random>
#include "GameConstants.hpp"

// Point template with concept constraints
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
struct Point {
    T x{};
    T y{};

    constexpr Point() = default;
    constexpr Point(T x_, T y_) : x(x_), y(y_) {}
};

namespace GameUtils {
    constexpr float ToRadians(float degrees);
    constexpr float ToDegrees(float radians);
    bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b);

    class Random {
    public:
        static float Percent();
        static float Range(float min, float max);
        static int Range(int min, int max);
        static float Angle();
    };

    template<Numeric T>
    [[nodiscard]] constexpr T Max(T a, T b) {
        return (a > b) ? a : b;
    }

    template<Numeric T>
    [[nodiscard]] constexpr T Min(T a, T b) {
        return (a < b) ? a : b;
    }

    [[nodiscard]] constexpr int FloatToInt(float value);
}

//// Resource management macros
//#define SAFE_DELETE(p) do { delete (p); (p) = nullptr; } while(0)
//#define SAFE_DELETE_ARRAY(p) do { delete[] (p); (p) = nullptr; } while(0)
//
//// Disallow copy/move operations macro
//#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
//    TypeName(const TypeName&) = delete; \
//    TypeName& operator=(const TypeName&) = delete; \
//    TypeName(TypeName&&) = delete; \
//    TypeName& operator=(TypeName&&) = delete