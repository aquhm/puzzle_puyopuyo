#include "BgParticleSystem.hpp"
#include "BgParticle.hpp"

#include "../../texture/ImageTexture.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/GameUtils.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>


void BgParticleSystem::Initialize(int particle_count, const BgParticleConfig& config) 
{
    config_ = config;
    SetParticleCount(particle_count);
}

void BgParticleSystem::SetParticleCount(int count) 
{
    particles_.clear();
    particles_.resize(count);
}

void BgParticleSystem::Update(float delta_time) 
{
    for (auto& particle : particles_) {
        UpdateParticle(particle, delta_time);
    }
}

void BgParticleSystem::Render(ImageTexture* effect_texture, const SDL_FRect& effect_rect) 
{
    if (!effect_texture)
    {
        return;
    }

    for (const auto& particle : particles_) 
    {
        if (particle.is_active) 
        {
            effect_texture->Render(particle.x, particle.y, &effect_rect, particle.angle);
        }
    }
}

void BgParticleSystem::UpdateParticle(BgParticle& particle, float delta_time)
{
    if (particle.is_active) 
    {
        particle.life_time += delta_time;

        if (particle.life_time > config_.life_time) 
        {
            particle.is_active = false;
        }
    }
    else 
    {
        if (particle.create_time > 0) 
        {
            particle.accumulated_time += delta_time;

            if (particle.accumulated_time >= particle.create_time) 
            {
                RespawnParticle(particle);
            }
        }
        else 
        {
            RespawnParticle(particle);
        }
    }
}

void GrasslandBgParticleSystem::UpdateParticle(BgParticle& particle, float delta_time) 
{
    if (particle.is_active) 
    {
        particle.angle += particle.rotation_velocity * delta_time;
        particle.curve_angle += particle.curve_period * delta_time;

        particle.angle = std::fmod(particle.angle, 360.0f);
        particle.curve_angle = std::fmod(particle.curve_angle, 360.0f);

        const float rad = particle.curve_angle * static_cast<float>(Constants::Math::PI) / 180.0f;
        particle.x += particle.amplitude * std::sin(rad);
        particle.y += delta_time * particle.down_velocity;
    }

    BgParticleSystem::UpdateParticle(particle, delta_time);
}

void GrasslandBgParticleSystem::RespawnParticle(BgParticle& particle)
{
    particle.is_active = true;
    particle.x = GameUtils::Random::Range(100.0f, 500.0f);
    particle.y = -100.0f;
    particle.create_time = GameUtils::Random::Range(1.5f, 5.5f);
    particle.down_velocity = GameUtils::Random::Range(150.0f, 250.0f);
    particle.rotation_velocity = GameUtils::Random::Range(35.0f, 85.0f);
    particle.amplitude = GameUtils::Random::Range(0.5f, 1.0f);
    particle.curve_period = GameUtils::Random::Range(50.0f, 100.0f);
    particle.effect_type = GameUtils::Random::Range(0, 1);
    particle.accumulated_time = 0.0f;
    particle.angle = 0.0f;
    particle.curve_angle = 0.0f;
}

void IcelandBgParticleSystem::UpdateParticle(BgParticle& particle, float delta_time)
{
    if (!particle.is_active) 
    {
        BgParticleSystem::UpdateParticle(particle, delta_time);
        return;
    }

    particle.angle += particle.rotation_velocity * delta_time;
    particle.angle = std::fmod(particle.angle, 360.0f);

    switch (current_state_) 
    {
    case State::Normal:
        particle.curve_angle += particle.curve_period * delta_time;
        particle.curve_angle = std::fmod(particle.curve_angle, 360.0f);
        particle.x += particle.amplitude * std::sin(particle.curve_angle * static_cast<float>(Constants::Math::PI / 180.0f));
        particle.y += delta_time * particle.down_velocity;
        break;

    case State::LeftBlizzard:
    case State::RightBlizzard: 
        const float angle = (current_state_ == State::LeftBlizzard) ? 230.0f : 310.0f;
        const float rad = angle * static_cast<float>(Constants::Math::PI / 180.0f);
        particle.x += config_.initial_velocity * std::cos(rad) * delta_time;
        particle.y += config_.initial_velocity * -std::sin(rad) * delta_time;
        break;
    }

    BgParticleSystem::UpdateParticle(particle, delta_time);
}

void IcelandBgParticleSystem::RespawnParticle(BgParticle& particle)
{
    particle.is_active = true;
    particle.x = GameUtils::Random::Range(100.0f, 500.0f);
    particle.y = -100.0f;
    particle.create_time = GameUtils::Random::Range(1.5f, 5.5f);
    particle.down_velocity = GameUtils::Random::Range(150.0f, 250.0f);
    particle.rotation_velocity = GameUtils::Random::Range(35.0f, 85.0f);
    particle.amplitude = GameUtils::Random::Range(0.5f, 1.0f);
    particle.curve_period = GameUtils::Random::Range(50.0f, 100.0f);
    particle.accumulated_time = 0.0f;
    particle.angle = 0.0f;
    particle.curve_angle = 0.0f;
}