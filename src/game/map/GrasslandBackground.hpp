#pragma once

#include "../core/GameBackground.hpp"
#include "../particles/ParticleSystem.hpp"

namespace bg {

    class GrasslandBackground final : public GameBackground {
    public:
        GrasslandBackground();
        ~GrasslandBackground() override;

        [[nodiscard]] bool Initialize() override;
        void Update(float deltaTime) override;
        void Render() override;
        void Release() override;

        void SetParticleCount(int count);

    private:
        std::unique_ptr<ImgTexture> effect_texture_;
        std::array<SDL_Rect, 2> effect_rects_;
        std::unique_ptr<GrasslandParticleSystem> particle_system_;

        [[nodiscard]] bool LoadEffectTextures();
    };

} // namespace bg