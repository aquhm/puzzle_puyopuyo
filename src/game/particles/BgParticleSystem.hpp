#pragma once

#include <vector>
#include <memory>
#include "BgParticle.hpp"


class ImageTexture;
struct SDL_FRect;


class BgParticleSystem 
{
public:
    BgParticleSystem() = default;
    virtual ~BgParticleSystem() = default;

    BgParticleSystem(const BgParticleSystem&) = delete;
    BgParticleSystem& operator=(const BgParticleSystem&) = delete;
    BgParticleSystem(BgParticleSystem&&) = delete;
    BgParticleSystem& operator=(BgParticleSystem&&) = delete;

    virtual void Initialize(int particle_count, const BgParticleConfig& config);
    virtual void Update(float delta_time);
    virtual void Render(ImageTexture* effect_texture, const SDL_FRect& effect_rect);

    void SetParticleCount(int count);
    [[nodiscard]] int GetParticleCount() const { return static_cast<int>(particles_.size()); }

protected:
    std::vector<BgParticle> particles_;
    BgParticleConfig config_;

    virtual void UpdateParticle(BgParticle& particle, float delta_time);
    virtual void RespawnParticle(BgParticle& particle) {}
};

class GrasslandBgParticleSystem : public BgParticleSystem
{
protected:
    void UpdateParticle(BgParticle& particle, float delta_time) override;
    void RespawnParticle(BgParticle& particle) override;
};

class IcelandBgParticleSystem : public BgParticleSystem
{
public:
    enum class State 
    {
        Normal,
        LeftBlizzard,
        RightBlizzard
    };

    void SetState(State new_state) { current_state_ = new_state; }

protected:
    void UpdateParticle(BgParticle& particle, float delta_time) override;
    void RespawnParticle(BgParticle& particle) override;

private:
    State current_state_{ State::Normal };
};
