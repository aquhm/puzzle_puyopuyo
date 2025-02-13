#pragma once

#include "GameBackground.hpp"
#include "../particles/ParticleSystem.hpp"

namespace bg {

    class IcelandBackground final : public GameBackground 
    {
    public:
        static constexpr float PARTICLE_ICE_VELOCITY = 500.0f;

        IcelandBackground();
        ~IcelandBackground() override;

        [[nodiscard]] bool Initialize() override;
        void Update(float deltaTime) override;
        void Render() override;
        void Release() override;

        void SetState(IcelandParticleSystem::State state);

    private:
        std::unique_ptr<ImgTexture> effect_texture_;
        SDL_Rect effect_rect_{};
        std::unique_ptr<IcelandParticleSystem> particle_system_;

        [[nodiscard]] bool LoadEffectTextures();
    };

} // namespace bg