#include "ParticleSystem.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../utils/Random.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>

namespace bg {

    void ParticleSystem::Initialize(int particle_count, const ParticleConfig& config) 
    {
        config_ = config;
        SetParticleCount(particle_count);
    }

    void ParticleSystem::SetParticleCount(int count) 
    {
        particles_.clear();
        particles_.resize(count);
    }

    void ParticleSystem::Update(float delta_time) 
    {
        for (auto& particle : particles_) {
            UpdateParticle(particle, delta_time);
        }
    }

    void ParticleSystem::Render(ImgTexture* effect_texture, const SDL_Rect& effect_rect) 
    {
        if (!effect_texture) return;

        for (const auto& particle : particles_) 
        {
            if (particle.is_active) 
            {
                effect_texture->Render(
                    static_cast<int>(particle.x),
                    static_cast<int>(particle.y),
                    &effect_rect,
                    particle.angle
                );
            }
        }
    }

    void ParticleSystem::UpdateParticle(Particle& particle, float delta_time) 
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

    void GrasslandParticleSystem::UpdateParticle(Particle& particle, float delta_time) 
    {
        if (particle.is_active) 
        {
            particle.angle += particle.rotation_velocity * delta_time;
            particle.curve_angle += particle.curve_period * delta_time;

            particle.angle = std::fmod(particle.angle, 360.0f);
            particle.curve_angle = std::fmod(particle.curve_angle, 360.0f);

            const float rad = particle.curve_angle * static_cast<float>(M_PI) / 180.0f;
            particle.x += particle.amplitude * std::sin(rad);
            particle.y += delta_time * particle.down_velocity;
        }

        ParticleSystem::UpdateParticle(particle, delta_time);
    }

    void GrasslandParticleSystem::RespawnParticle(Particle& particle) 
    {
        particle.is_active = true;
        particle.x = Random::Range(100.0f, 500.0f);
        particle.y = -100.0f;
        particle.create_time = Random::Range(1.5f, 5.5f);
        particle.down_velocity = Random::Range(150.0f, 250.0f);
        particle.rotation_velocity = Random::Range(35.0f, 85.0f);
        particle.amplitude = Random::Range(0.5f, 1.0f);
        particle.curve_period = Random::Range(50.0f, 100.0f);
        particle.effect_type = Random::Range(0, 1);
        particle.accumulated_time = 0.0f;
        particle.angle = 0.0f;
        particle.curve_angle = 0.0f;
    }

    void IcelandParticleSystem::UpdateParticle(Particle& particle, float delta_time) 
    {
        if (!particle.is_active) 
        {
            ParticleSystem::UpdateParticle(particle, delta_time);
            return;
        }

        particle.angle += particle.rotation_velocity * delta_time;
        particle.angle = std::fmod(particle.angle, 360.0f);

        switch (current_state_) 
        {
        case State::Normal:
            particle.curve_angle += particle.curve_period * delta_time;
            particle.curve_angle = std::fmod(particle.curve_angle, 360.0f);
            particle.x += particle.amplitude * std::sin(particle.curve_angle * static_cast<float>(M_PI) / 180.0f);
            particle.y += delta_time * particle.down_velocity;
            break;

        case State::LeftBlizzard:
        case State::RightBlizzard: {
            const float angle = (current_state_ == State::LeftBlizzard) ? 230.0f : 310.0f;
            const float rad = angle * static_cast<float>(M_PI) / 180.0f;
            particle.x += config_.initial_velocity * std::cos(rad) * delta_time;
            particle.y += config_.initial_velocity * -std::sin(rad) * delta_time;
            break;
        }
        }

        ParticleSystem::UpdateParticle(particle, delta_time);
    }

    void IcelandParticleSystem::RespawnParticle(Particle& particle) 
    {
        particle.is_active = true;
        particle.x = Random::Range(100.0f, 500.0f);
        particle.y = -100.0f;
        particle.create_time = Random::Range(1.5f, 5.5f);
        particle.down_velocity = Random::Range(150.0f, 250.0f);
        particle.rotation_velocity = Random::Range(35.0f, 85.0f);
        particle.amplitude = Random::Range(0.5f, 1.0f);
        particle.curve_period = Random::Range(50.0f, 100.0f);
        particle.accumulated_time = 0.0f;
        particle.angle = 0.0f;
        particle.curve_angle = 0.0f;
    }

} // namespace bg