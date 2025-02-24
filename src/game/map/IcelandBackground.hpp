#pragma once

/*
*
* 설명: 얼음 컨셉의 맵을 구성하는 파티클 시스템과 맵 Class
*
*/


#include "GameBackground.hpp"
#include "BackgroundParticleSystem.hpp"

#include "../particles/BgParticleSystem.hpp"



constexpr float	PARTICLE_ICE_VELOCITY = 500.0f;

class IcelandParticleSystem final : public BackgroundParticleSystem
{
public:

    enum class State
    {
        Normal,
        LeftBlizzard,
        RightBlizzard
    };

    void SetState(State new_state);
    [[nodiscard]] State GetState() const { return current_state_; }

protected:
    void SpawnParticle(BGParticle& particle) override;
    void UpdateParticle(BGParticle& particle, float deltaTime) override;

private:
    State current_state_{ State::Normal };
    SDL_FPoint direction_{ 0.0f, 0.0f };

    void UpdateDirectionVector();
    void UpdateNormalState(BGParticle& particle, float deltaTime);
    void UpdateBlizzardState(BGParticle& particle, float deltaTime, float angle);
};

class IcelandBackground final : public GameBackground
{
public:
    static constexpr float PARTICLE_ICE_VELOCITY = 500.0f;
    static constexpr float STATE_CHANGE_TIME = 10.0f;
    static constexpr float NORMAL_STATE_TIME = 15.0f;

    IcelandBackground();
    ~IcelandBackground() override;

    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    void SetState(IcelandParticleSystem::State state);

private:
    std::shared_ptr<ImageTexture> effect_texture_;
    SDL_FRect effect_rect_{};
    std::unique_ptr<IcelandParticleSystem> particle_system_;

    [[nodiscard]] bool LoadEffectTextures();
};