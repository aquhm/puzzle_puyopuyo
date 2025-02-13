#pragma once
#include <numbers>

namespace math 
{

    constexpr double PI = std::numbers::pi;
    constexpr float PI_F = std::numbers::pi_v<float>;

    constexpr float DEGREES_TO_RADIANS = PI_F / 180.0f;
    constexpr float RADIANS_TO_DEGREES = 180.0f / PI_F;

    inline float ToRadians(float degrees) 
    {
        return degrees * DEGREES_TO_RADIANS;
    }

    inline float ToDegrees(float radians) 
    {
        return radians * RADIANS_TO_DEGREES;
    }
}