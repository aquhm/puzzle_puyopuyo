#include "ParticleManager.hpp"
#include "../../game/effect/ParticleContainer.hpp"
#include "../../texture/ImageTexture.hpp"

#include <cassert>
#include <stdexcept>
#include "../../utils/Logger.hpp"


ParticleManager::~ParticleManager() 
{
    if (is_initialized_) 
    {
        Release();
    }
}

bool ParticleManager::Initialize() 
{
    if (is_initialized_) 
    {
        return true;
    }

    try 
    {
        containers_.clear();
        textures_.clear();
        is_draw_enabled_ = true;
        is_initialized_ = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "ParticleManager initialization failed: %s", e.what());
        return false;
    }
}

void ParticleManager::Update(float deltaTime) 
{
    if (!is_initialized_)
    {
        return;
    }

    for (auto it = containers_.begin(); it != containers_.end();) 
    {
        if ((*it)->GetAliveParticleCount() == 0) 
        {
            it = containers_.erase(it);
        }
        else 
        {
            (*it)->Update(deltaTime);
            ++it;
        }
    }
}

void ParticleManager::Render() 
{

}

void ParticleManager::RenderForPlayer(uint8_t playerId) 
{
    if (!is_initialized_ || !is_draw_enabled_)
    {
        return;
    }

    for (const auto& container : containers_) 
    {
        if (container->GetPlayerID() == playerId) 
        {
            container->Render();
        }
    }
}

void ParticleManager::AddParticleContainer(const std::shared_ptr<ParticleContainer>&& container)
{
    if (!is_initialized_) 
    {
        throw std::runtime_error("ParticleManager not initialized");
    }

    assert(container);
    if (!container->InitializeParticles()) 
    {
        return;
    }
    containers_.push_back(std::move(container));
}

void ParticleManager::AddParticleContainer(const std::shared_ptr<ParticleContainer>&& container, const SDL_FPoint& position)
{
    if (!is_initialized_) 
    {
        throw std::runtime_error("ParticleManager not initialized");
    }

    assert(container);
    container->SetPosition(position);

    if (!container->InitializeParticles()) 
    {
        return;
    }
    containers_.push_back(std::move(container));
}

void ParticleManager::RemoveParticleContainer(const ParticleContainer& container) 
{
    if (!is_initialized_)
    {
        return;
    }

    containers_.remove_if([&container](const auto& ptr) 
        {
            return ptr.get() == &container;
        });
}

std::shared_ptr<ImageTexture> ParticleManager::FindParticleTexture(const std::string& name) 
{
    if (!is_initialized_)
    {
        return nullptr;
    }

    auto it = textures_.find(name);
    return (it != textures_.end()) ? it->second : nullptr;
}

void ParticleManager::Release() 
{
    ClearAllResources();
    is_initialized_ = false;
}

void ParticleManager::ClearAllResources() 
{
    containers_.clear();
    textures_.clear();
    is_draw_enabled_ = true;
}