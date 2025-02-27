#include "GrasslandBackground.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameUtils.hpp"
#include <format>
#include "../../utils/Logger.hpp"
#include "../../utils/PathUtil.hpp"

void GrasslandParticleSystem::SpawnParticle(BGParticle& particle)
{
    particle.is_active = true;
    particle.x = GameUtils::Random::Range(bounds_.x, bounds_.x + bounds_.w);
    particle.y = -100.0f;
    particle.create_time = GameUtils::Random::Range(3.5f, 6.5f);
    particle.down_velocity = GameUtils::Random::Range(150.0f, 250.0f);
    particle.rotation_velocity = GameUtils::Random::Range(MIN_ROTATION_SPEED, MAX_ROTATION_SPEED);
    particle.amplitude = GameUtils::Random::Range(MIN_AMPLITUDE, MAX_AMPLITUDE);
    particle.curve_period = GameUtils::Random::Range(50.0f, 100.0f);
    particle.effect_type = GameUtils::Random::Range(0, 1);
}

void GrasslandParticleSystem::UpdateParticle(BGParticle& particle, float deltaTime)
{
    particle.angle += particle.rotation_velocity * deltaTime;
    particle.curve_angle += particle.curve_period * deltaTime;

    UpdateParticleMovement(particle, deltaTime);
}

void GrasslandParticleSystem::UpdateParticleMovement(BGParticle& particle, float deltaTime)
{
    if (particle.curve_angle > 360.0f)
    {
        particle.curve_angle -= 360.0f;
    }

    particle.x += particle.amplitude * std::sin(GameUtils::ToRadians(particle.curve_angle));
    particle.y += deltaTime * particle.down_velocity;
}

// GrasslandBackground Implementation
GrasslandBackground::GrasslandBackground()
{
    map_index_ = static_cast<int16_t>(BackgroundType::GrassLand);
    particle_system_ = std::make_unique<GrasslandParticleSystem>();
}

GrasslandBackground::~GrasslandBackground() = default;

bool GrasslandBackground::Initialize()
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
    config.initial_velocity = GrasslandParticleSystem::INITIAL_VELOCITY;
    config.life_time = GrasslandParticleSystem::LIFE_TIME;
    config.min_rotation_speed = GrasslandParticleSystem::MIN_ROTATION_SPEED;
    config.max_rotation_speed = GrasslandParticleSystem::MAX_ROTATION_SPEED;
    config.min_amplitude = GrasslandParticleSystem::MIN_AMPLITUDE;
    config.max_amplitude = GrasslandParticleSystem::MAX_AMPLITUDE;
    config.spawn_rate = GrasslandParticleSystem::SPAWN_RATE;
    config.max_particles = GrasslandParticleSystem::DEFAULT_PARTICLE_COUNT;

    particle_system_->Initialize(GrasslandParticleSystem::DEFAULT_PARTICLE_COUNT, config);

    return true;
}

bool GrasslandBackground::LoadEffectTextures()
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

        const auto half_width = static_cast<float>(effect_texture_->GetWidth()) / 2.0f;
        const auto height = static_cast<float>(effect_texture_->GetHeight());

        effect_rects_[0] = SDL_FRect{ 0.0f, 0.0f, half_width, height };
        effect_rects_[1] = SDL_FRect{ half_width, 0.0f, half_width, height };

        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load effect texture: %s", e.what());
        return false;
    }
}

void GrasslandBackground::Update(float deltaTime)
{
    GameBackground::Update(deltaTime);

    if (particle_system_)
    {
        particle_system_->Update(deltaTime);
    }
}

void GrasslandBackground::Render()
{
    GameBackground::Render();

    if (effect_texture_ && particle_system_)
    {
        particle_system_->Render(effect_texture_.get(), effect_rects_);
    }
}

void GrasslandBackground::Release()
{
    GameBackground::Release();
    effect_texture_.reset();
    particle_system_.reset();
}

void GrasslandBackground::SetParticleCount(size_t count)
{
    if (particle_system_)
    {
        particle_system_->SetParticleCount(count);
    }
}

size_t GrasslandBackground::GetParticleCount() const
{
    return particle_system_ ? particle_system_->GetParticleCount() : 0;
}