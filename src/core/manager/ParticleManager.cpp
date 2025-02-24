#include "ParticleManager.hpp"
#include "../../game/effect/ParticleContainer.hpp"
#include "../../texture/ImageTexture.hpp"

#include <cassert>
#include <stdexcept>


ParticleManager::~ParticleManager() 
{
    if (isInitialized_) 
    {
        Release();
    }
}

bool ParticleManager::Initialize() 
{
    if (isInitialized_) 
    {
        return true;
    }

    try 
    {
        containers_.clear();
        textures_.clear();
        isDrawEnabled_ = true;
        isInitialized_ = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ParticleManager initialization failed: %s", e.what());
        return false;
    }
}

void ParticleManager::Update(float deltaTime) 
{
    if (!isInitialized_)
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
    if (!isInitialized_ || !isDrawEnabled_)
    {
        return;
    }

    for (const auto& container : containers_) 
    {
        container->Render();
    }
}

void ParticleManager::RenderForPlayer(uint8_t playerId) 
{
    if (!isInitialized_ || !isDrawEnabled_)
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

void ParticleManager::AddParticleContainer(std::unique_ptr<ParticleContainer> container) 
{
    if (!isInitialized_) 
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

void ParticleManager::AddParticleContainer(std::unique_ptr<ParticleContainer> container, const SDL_FPoint& position)
{
    if (!isInitialized_) 
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
    if (!isInitialized_)
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
    if (!isInitialized_)
    {
        return nullptr;
    }

    auto it = textures_.find(name);
    return (it != textures_.end()) ? it->second : nullptr;
}

void ParticleManager::Release() 
{
    ClearAllResources();
    isInitialized_ = false;
}

void ParticleManager::ClearAllResources() 
{
    containers_.clear();
    textures_.clear();
    isDrawEnabled_ = true;
}