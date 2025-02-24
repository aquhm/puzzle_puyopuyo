#pragma once

/**
 *
 * 설명: 각종 유틸리티 함수 모음
 *
 */

#include <SDL3/SDL.h>

#include "../core/common/constants/Constants.hpp"

#include <concepts>
#include <random>
#include <type_traits>

#include <cassert>

template<typename T>
concept NumericType = std::is_arithmetic_v<T>;

template<NumericType T>
struct Point {
    T x{};
    T y{};

    constexpr Point() = default;
    constexpr Point(T x_, T y_) : x(x_), y(y_) {}
};

namespace GameUtils 
{
    namespace Random
    {
        inline std::mt19937& GetRandomEngine() 
        {
            static std::random_device rd;
            static std::mt19937 engine(rd());
            return engine;
        }

        template<typename T>
        inline T Range(T min, T max) 
        {
            assert(min <= max);
            if constexpr (std::is_floating_point_v<T>) 
            {
                std::uniform_real_distribution<T> dist(min, max);
                return dist(GetRandomEngine());
            }
            else 
            {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(GetRandomEngine());
            }
        }

        inline float Percent() 
        {
            return Range(0.0f, 1.0f);
        }

        // 0 ~ 2π 라디안 사이의 각도 난수
        inline float Angle() 
        {
            return Range(0.0f, 2.0f * static_cast<float>(Constants::Math::PI));
        }
    }

    constexpr float ToRadians(float degrees)
    {
        return degrees * (static_cast<float>(Constants::Math::PI) / 180.0f);
    }

    constexpr float ToDegrees(float radians)
    {
        return radians * (180.0f / static_cast<float>(Constants::Math::PI));
    }

    template<NumericType T>
    [[nodiscard]] constexpr T Max(T a, T b)
    {
        return (a > b) ? a : b;
    }

    template<NumericType T>
    [[nodiscard]] constexpr T Min(T a, T b) 
    {
        return (a < b) ? a : b;
    }

    [[nodiscard]] constexpr int FloatToInt(float value)
    {
        int i = static_cast<int>(value);
        return (value - i >= 0.999f) ? ++i : i;
    }    
    

    inline bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b)
    {
        return !(a.y + a.h < b.y ||
            a.y > b.y + b.h ||
            a.x + a.w < b.x ||
            a.x > b.x + b.w);
    }

    // 라디안 각도를 계산하는 함수
    inline float CalculateAngle(float deltaY, float deltaX)
    {
        return std::atan2(deltaY, deltaX);
    }

    // 라디안 각도로부터 방향 벡터를 계산하는 함수
    inline void SetDirectionVector(float angle, float& x, float& y)
    {
        x = std::cos(angle);
        y = -std::sin(angle);
    }

    inline float CalculateAngleInDegrees(float deltaY, float deltaX) 
    {
        return std::atan2(deltaY, deltaX) * Constants::Math::RADIANS_TO_DEGREES;
    }

    inline void SetDirectionVectorFromDegrees(float angleDegrees, float& x, float& y) 
    {
        float angleRadians = angleDegrees * Constants::Math::DEGREES_TO_RADIANS;
        x = std::cos(angleRadians);
        y = -std::sin(angleRadians);
    }
}