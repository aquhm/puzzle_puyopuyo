#include "BackgroundParticleSystem.hpp"
#include "../../texture/ImageTexture.hpp"
#include <algorithm>
#include <SDL3/SDL.h>


bool BackgroundParticleSystem::Initialize(size_t particle_count, const BGParticleConfig& config) 
{
    config_ = config;
    SetParticleCount(particle_count);
    return true;
}

void BackgroundParticleSystem::Update(float delta_time) 
{
    spawn_timer_ += delta_time;

    if (spawn_timer_ >= config_.spawn_rate) 
    {
        spawn_timer_ = 0.0f;

        auto it = std::find_if(particles_.begin(), particles_.end(),
            [](const BGParticle& p) 
            { 
                return !p.is_active; 
            });

        if (it != particles_.end()) 
        {
            SpawnParticle(*it);
        }
    }

    for (auto& particle : particles_) 
    {
        if (particle.is_active) 
        {
            UpdateParticle(particle, delta_time);

            particle.life_time += delta_time;

            if (particle.life_time >= config_.life_time) 
            {
                particle.is_active = false;
            }
        }
    }
}

void BackgroundParticleSystem::Render(ImageTexture* texture, const std::array<SDL_FRect, 2>& effect_rects)
{
    if (!texture)
    {
        return;
    }

    for (const auto& particle : particles_)
    {
        if (particle.is_active && particle.effect_type < effect_rects.size())
        {
            texture->Render(particle.x, particle.y, &effect_rects[particle.effect_type], particle.angle);
        }
    }
}

void BackgroundParticleSystem::Reset() 
{
    for (auto& particle : particles_) 
    {
        particle.is_active = false;
    }
    spawn_timer_ = 0.0f;
}

void BackgroundParticleSystem::SetParticleCount(size_t count) 
{
    particles_.clear();
    particles_.resize(count);
}

