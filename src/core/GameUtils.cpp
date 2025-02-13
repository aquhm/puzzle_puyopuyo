#include "GameUtils.hpp"
#include <cassert>

namespace GameUtils 
{
    constexpr float ToRadians(float degrees) 
    {
        return degrees * (GameConstants::PI / 180.0f);
    }

    constexpr float ToDegrees(float radians) 
    {
        return radians * (180.0f / GameConstants::PI);
    }

    float Random::Percent() 
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        return dis(gen);
    }

    float Random::Range(float min, float max) 
    {
        assert(min <= max);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(min, max);
        return dis(gen);
    }

    int Random::Range(int min, int max) 
    {
        assert(min <= max);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(min, max);
        return dis(gen);
    }

    float Random::Angle() 
    {
        return Range(0.0f, 2.0f * GameConstants::PI);
    }

    bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b) 
    {
        return !(a.y + a.h < b.y ||
            a.y > b.y + b.h ||
            a.x + a.w < b.x ||
            a.x > b.x + b.w);
    }

    constexpr int FloatToInt(float value) 
    {
        int i = static_cast<int>(value);
        return (value - i >= 0.999f) ? ++i : i;
    }
}