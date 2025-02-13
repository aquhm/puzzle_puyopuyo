#pragma once

#include "Particle.hpp"
#include <vector>
#include <memory>

class ImgTexture;
struct SDL_Rect;

namespace bg {

    class ParticleSystem 
    {
    public:
        ParticleSystem() = default;
        virtual ~ParticleSystem() = default;

        // Delete copy/move operations
        ParticleSystem(const ParticleSystem&) = delete;
        ParticleSystem& operator=(const ParticleSystem&) = delete;
        ParticleSystem(ParticleSystem&&) = delete;
        ParticleSystem& operator=(ParticleSystem&&) = delete;

        virtual void Initialize(int particle_count, const ParticleConfig& config);
        virtual void Update(float delta_time);
        virtual void Render(ImgTexture* effect_texture, const SDL_Rect& effect_rect);

        void SetParticleCount(int count);
        [[nodiscard]] int GetParticleCount() const { return static_cast<int>(particles_.size()); }

    protected:
        std::vector<Particle> particles_;
        ParticleConfig config_;

        virtual void UpdateParticle(Particle& particle, float delta_time);
        virtual void RespawnParticle(Particle& particle);
    };

    // Specialized particle systems
    class GrasslandParticleSystem : public ParticleSystem 
    {
    protected:
        void UpdateParticle(Particle& particle, float delta_time) override;
        void RespawnParticle(Particle& particle) override;
    };

    class IcelandParticleSystem : public ParticleSystem 
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
        void UpdateParticle(Particle& particle, float delta_time) override;
        void RespawnParticle(Particle& particle) override;

    private:
        State current_state_{ State::Normal };
    };
}