#include "IcelandBackground.hpp"
#include "../../texture/ImageTexture.hpp"
#include <format>

namespace bg {

    IcelandBackground::IcelandBackground() {
        map_index_ = static_cast<int16_t>(BackgroundType::IceLand);
        particle_system_ = std::make_unique<IcelandParticleSystem>();
    }

    IcelandBackground::~IcelandBackground() = default;

    bool IcelandBackground::Initialize() {
        if (!GameBackground::Initialize()) {
            return false;
        }

        if (!LoadEffectTextures()) {
            return false;
        }

        ParticleConfig config;
        config.initial_velocity = PARTICLE_ICE_VELOCITY;
        config.life_time = 7.0f;
        config.min_rotation_speed = 35.0f;
        config.max_rotation_speed = 85.0f;
        config.min_amplitude = 0.5f;
        config.max_amplitude = 1.0f;

        particle_system_->Initialize(12, config);
        particle_system_->SetState(IcelandParticleSystem::State::Normal);

        return true;
    }

    bool IcelandBackground::LoadEffectTextures() 
    {
        effect_texture_ = std::make_unique<ImgTexture>();

        if (!effect_texture_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create effect texture");
            return false;
        }

        auto effect_path = std::format("{}{:02d}/op{:02d}_00.png",
            file_path_, map_index_, map_index_);

        if (!effect_texture_->LoadTexture(effect_path)) 
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load effect texture");
            return false;
        }

        // Set up effect rectangle
        effect_rect_ = { 0, 0, 128, 128 };

        return true;
    }

    void IcelandBackground::Update(float delta_time) 
    {
        GameBackground::Update(delta_time);

        particle_system_->Update(delta_time);

        // State transition logic
        if (particle_system_) 
        {
            const float STATE_CHANGE_TIME = 10.0f;
            const float NORMAL_STATE_TIME = 15.0f;

            if (accumulated_time_ >= STATE_CHANGE_TIME)
            {
                accumulated_time_ = 0.0f;

                switch (particle_system_->GetState()) 
                {
                case IcelandParticleSystem::State::Normal:
                    if (accumulated_time_ >= NORMAL_STATE_TIME) 
                    {
                        SetState(IcelandParticleSystem::State::LeftBlizzard);
                    }
                    break;

                case IcelandParticleSystem::State::LeftBlizzard:
                    SetState(IcelandParticleSystem::State::RightBlizzard);
                    break;

                case IcelandParticleSystem::State::RightBlizzard:
                    SetState(IcelandParticleSystem::State::Normal);
                    break;
                }
            }
        }
    }

    void IcelandBackground::Render() 
    {
        GameBackground::Render();

        if (effect_texture_) 
        {
            particle_system_->Render(effect_texture_.get(), effect_rect_);
        }
    }

    void IcelandBackground::Release() 
    {
        GameBackground::Release();
        effect_texture_.reset();
        particle_system_.reset();
    }

    void IcelandBackground::SetState(IcelandParticleSystem::State state) 
    {
        if (particle_system_) 
        {
            particle_system_->SetState(state);
        }
    }

} // namespace bg