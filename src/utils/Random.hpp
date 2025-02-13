#pragma once
#include <random>

class Random 
{
public:
    
    static float Range(float min, float max) 
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }

    static int Range(int min, int max) 
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }

private:
    Random() = delete;
};