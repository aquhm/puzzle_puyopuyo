#pragma once

#include <cstdint>

struct BgParticle
{
    bool is_active{ false };
    uint8_t effect_type{ 0 };
    float x{ 0.0f }, y{ 0.0f };
    float rotation_velocity{ 0.0f };
    float down_velocity{ 0.0f };
    float create_time{ 0.0f };
    float life_time{ 0.0f };
    float accumulated_time{ 0.0f };
    float angle{ 0.0f };
    float curve_angle{ 0.0f };
    float curve_period{ 0.0f };
    float amplitude{ 0.0f };
};

struct BgParticleConfig 
{
    float initial_velocity{ 0.0f };
    float life_time{ 7.0f };
    float min_rotation_speed{ 35.0f };
    float max_rotation_speed{ 85.0f };
    float min_amplitude{ 0.5f };
    float max_amplitude{ 1.0f };
};
