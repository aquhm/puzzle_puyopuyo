#include "IcelandBackground.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameUtils.hpp"
#include <format>
#include "../../utils/Logger.hpp"
#include "../../utils/PathUtil.hpp"

void IcelandParticleSystem::SetState(State new_state)
{
    if (current_state_ == new_state)
    {
        return;
    }

    current_state_ = new_state;
    UpdateDirectionVector();
}

void IcelandParticleSystem::UpdateDirectionVector()
{
    switch (current_state_)
    {
    case State::Normal:
        direction_ = { 0.0f, 1.0f };
        break;

    case State::LeftBlizzard:
        direction_.x = PARTICLE_ICE_VELOCITY * std::cos(GameUtils::ToRadians(230.0f));
        direction_.y = PARTICLE_ICE_VELOCITY * -std::sin(GameUtils::ToRadians(230.0f));
        break;

    case State::RightBlizzard:
        direction_.x = PARTICLE_ICE_VELOCITY * std::cos(GameUtils::ToRadians(310.0f));
        direction_.y = PARTICLE_ICE_VELOCITY * -std::sin(GameUtils::ToRadians(310.0f));
        break;
    }
}

void IcelandParticleSystem::SpawnParticle(BGParticle& particle)
{
    particle.is_active = true;
    particle.x = GameUtils::Random::Range(100.0f, 500.0f);
    particle.y = -100.0f;
    particle.create_time = GameUtils::Random::Range(1.5f, 5.5f);
    particle.down_velocity = GameUtils::Random::Range(150.0f, 250.0f);
    particle.rotation_velocity = GameUtils::Random::Range(35.0f, 85.0f);
    particle.amplitude = GameUtils::Random::Range(0.5f, 1.0f);
    particle.curve_period = GameUtils::Random::Range(50.0f, 100.0f);
}

void IcelandParticleSystem::UpdateParticle(BGParticle& particle, float deltaTime)
{
    particle.angle += particle.rotation_velocity * deltaTime;

    switch (current_state_)
    {
    case State::Normal:
        UpdateNormalState(particle, deltaTime);
        break;
    case State::LeftBlizzard:
    case State::RightBlizzard:
        UpdateBlizzardState(particle, deltaTime, current_state_ == State::LeftBlizzard ? 230.0f : 310.0f);
        break;
    }
}

void IcelandParticleSystem::UpdateNormalState(BGParticle& particle, float deltaTime)
{
    particle.curve_angle += particle.curve_period * deltaTime;

    if (particle.curve_angle > 360.0f)
    {
        particle.curve_angle -= 360.0f;
    }

    particle.x += particle.amplitude * std::sin(GameUtils::ToRadians(particle.curve_angle));
    particle.y += deltaTime * particle.down_velocity;
}

void IcelandParticleSystem::UpdateBlizzardState(BGParticle& particle, float deltaTime, float angle)
{
    particle.x += direction_.x * deltaTime;
    particle.y += direction_.y * deltaTime;
}

IcelandBackground::IcelandBackground()
{
    map_index_ = static_cast<int16_t>(BackgroundType::IceLand);
    particle_system_ = std::make_unique<IcelandParticleSystem>();
}

IcelandBackground::~IcelandBackground() = default;

bool IcelandBackground::Initialize()
{
    if (!GameBackground::Initialize())
    {
        return false;
    }

    if (!LoadEffectTextures())
    {
        return false;
    }

    BGParticleConfig config;
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
    try
    {
        std::string bgPath = PathUtil::GetBgPath();

        auto effect_path = std::format("{}/bg{:02d}/op{:02d}_00.png", bgPath, map_index_, map_index_);
        effect_texture_ = ImageTexture::Create(effect_path);

        if (!effect_texture_)
        {
            throw std::runtime_error("Failed to load effect texture");
        }

        effect_rect_ = { 0.0f, 0.0f, 128.0f, 128.0f };
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load effect texture: %s", e.what());
        return false;
    }
}

void IcelandBackground::Update(float deltaTime)
{
    GameBackground::Update(deltaTime);

    if (particle_system_)
    {
        particle_system_->Update(deltaTime);

        accumulated_time_ += deltaTime;
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

    if (effect_texture_ && particle_system_)
    {
        std::array<SDL_FRect, 2> effect_rects = { effect_rect_, effect_rect_ };
        particle_system_->Render(effect_texture_.get(), effect_rects);
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