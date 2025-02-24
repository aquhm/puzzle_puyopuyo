#pragma once

/**
 *
 * 설명: 맵 배경에 따라 움직이는 오브젝트 묘사를 위한 배경 전용 파티클 시스템
 *
 */

#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <array>


class ImageTexture;

struct BGParticleConfig 
{
    float initial_velocity{ 70.0f };
    float life_time{ 7.0f };
    float min_rotation_speed{ 35.0f };
    float max_rotation_speed{ 85.0f };
    float min_amplitude{ 0.5f };
    float max_amplitude{ 1.0f };
    float spawn_rate{ 1.0f };
    size_t max_particles{ 50 };
};

struct BGParticle 
{
    bool is_active{ false };
    uint8_t effect_type{ 0 };
    float x{ 0.0f }, y{ 0.0f };
    float velocity_x{ 0.0f }, velocity_y{ 0.0f };
    float rotation_velocity{ 0.0f };
    float down_velocity{ 0.0f };
    float create_time{ 0.0f };
    float life_time{ 0.0f };
    float accumulated_time{ 0.0f };
    float angle{ 0.0f };
    float curve_angle{ 0.0f };
    float curve_period{ 0.0f };
    float amplitude{ 0.0f };
    float scale{ 1.0f };
};

struct BGParticleState 
{
    virtual ~BGParticleState() = default;
};

class BackgroundParticleSystem 
{
public:
    BackgroundParticleSystem() = default;
    virtual ~BackgroundParticleSystem() = default;

    virtual bool Initialize(size_t particle_count, const BGParticleConfig& config);
    virtual void Update(float delta_time);
    virtual void Render(ImageTexture* texture, const std::array<SDL_FRect, 2>& effect_rects);
    virtual void Reset();

    void SetParticleCount(size_t count);
    [[nodiscard]] size_t GetParticleCount() const { return particles_.size(); }
    void SetBounds(const SDL_FRect& bounds) { bounds_ = bounds; }

protected:
    virtual void SpawnParticle(BGParticle& particle) = 0;
    virtual void UpdateParticle(BGParticle& particle, float delta_time) = 0;

protected:

    std::vector<BGParticle> particles_;
    SDL_FRect bounds_{ 0, 0, 0, 0 };
    BGParticleConfig config_;
    float spawn_timer_{ 0.0f };
};

