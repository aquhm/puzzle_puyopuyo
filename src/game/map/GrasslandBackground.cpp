#include "GrasslandBackground.hpp"
#include "../../core/ImgTexture.hpp"
#include <format>

namespace bg {

    GrasslandBackground::GrasslandBackground() {
        map_index_ = static_cast<int16_t>(BackgroundType::GrassLand);
        particle_system_ = std::make_unique<GrasslandParticleSystem>();
    }

    GrasslandBackground::~GrasslandBackground() = default;

    bool GrasslandBackground::Initialize() {
        if (!GameBackground::Initialize()) {
            return false;
        }

        if (!LoadEffectTextures()) {
            return false;
        }

        ParticleConfig config;
        config.initial_velocity = 70.0f;
        config.life_time = 10.0f;
        config.min_rotation_speed = 45.0f;
        config.max_rotation_speed = 65.0f;
        config.min_amplitude = 0.5f;
        config.max_amplitude = 1.5f;

        particle_system_->Initialize(6, config);

        return true;
    }

    bool GrasslandBackground::LoadEffectTextures() {
        effect_texture_ = std::make_unique<ImgTexture>();

        if (!effect_texture_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create effect texture");
            return false;
        }

        auto effect_path = std::format("{}{:02d}/op{:02d}_00.png",
            file_path_, map_index_, map_index_);

        if (!effect_texture_->LoadTexture(effect_path)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load effect texture");
            return false;
        }

        // Set up effect rectangles
        const int half_width = effect_texture_->GetWidth() / 2;
        const int height = effect_texture_->GetHeight();

        effect_rects_[0] = { 0, 0, half_width, height };
        effect_rects_[1] = { half_width, 0, half_width, height };

        return true;
    }

    void GrasslandBackground::Update(float delta_time) {
        GameBackground::Update(delta_time);
        particle_system_->Update(delta_time);
    }

    void GrasslandBackground::Render() {
        GameBackground::Render();

        if (effect_texture_) {
            for (size_t i = 0; i < effect_rects_.size(); ++i) {
                particle_system_->Render(effect_texture_.get(), effect_rects_[i]);
            }
        }
    }

    void GrasslandBackground::Release() {
        GameBackground::Release();
        effect_texture_.reset();
        particle_system_.reset();
    }

    void GrasslandBackground::SetParticleCount(int count) {
        if (particle_system_) {
            particle_system_->SetParticleCount(count);
        }
    }

} // namespace bg