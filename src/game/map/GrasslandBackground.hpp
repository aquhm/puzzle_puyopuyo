#pragma once
/**
 *
 * 설명: 초원 컨셉의 맵을 구성하는 파티클 시스템과 맵 Class
 *
 */

#include "GameBackground.hpp"
#include "BackgroundParticleSystem.hpp"
#include "../../core/GameUtils.hpp"

class GrasslandParticleSystem final : public BackgroundParticleSystem
{
public:
    static constexpr float INITIAL_VELOCITY = 70.0f;
    static constexpr float LIFE_TIME = 10.0f;
    static constexpr float MIN_ROTATION_SPEED = 45.0f;
    static constexpr float MAX_ROTATION_SPEED = 65.0f;
    static constexpr float MIN_AMPLITUDE = 0.5f;
    static constexpr float MAX_AMPLITUDE = 1.5f;
    static constexpr float SPAWN_RATE = 1.0f;
    static constexpr int DEFAULT_PARTICLE_COUNT = 6;

protected:
    void SpawnParticle(BGParticle& particle) override;
    void UpdateParticle(BGParticle& particle, float deltaTime) override;

private:
    void UpdateParticleMovement(BGParticle& particle, float deltaTime);
};

class GrasslandBackground final : public GameBackground
{
public:
    GrasslandBackground();
    ~GrasslandBackground() override;

    GrasslandBackground(const GrasslandBackground&) = delete;
    GrasslandBackground& operator=(const GrasslandBackground&) = delete;
    GrasslandBackground(GrasslandBackground&&) = delete;
    GrasslandBackground& operator=(GrasslandBackground&&) = delete;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    void SetParticleCount(size_t count);
    [[nodiscard]] size_t GetParticleCount() const;

private:
    [[nodiscard]] bool LoadEffectTextures();

private:
    std::shared_ptr<ImageTexture> effect_texture_;
    std::array<SDL_FRect, 2> effect_rects_;
    std::unique_ptr<GrasslandParticleSystem> particle_system_;
};