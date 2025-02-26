#include "ResourceManager.hpp"
#include "../GameApp.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <format>
#include "../../utils/Logger.hpp"

bool ResourceManager::Initialize()
{
    try
    {
        resources_.clear();
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "ResourceManager initialization failed: %s", e.what());
        return false;
    }
}

void ResourceManager::Update(float deltaTime)
{
}

void ResourceManager::Release()
{
    for (auto& [type, container] : resources_)
    {
        for (auto& [path, resource] : container)
        {
            if (resource)
            {
                resource->Unload();
            }
        }
        container.clear();
    }
    resources_.clear();
    renderer_ = nullptr;
}

bool ResourceManager::IsResourceLoaded(const std::string& filename) const
{
    for (const auto& [type, container] : resources_)
    {
        if (container.contains(filename))
        {
            return true;
        }
    }
    return false;
}


SDL_Texture* ResourceManager::CreateTextureFromSurface(SDL_Surface* surface) const
{
    if (!surface)
    {
        throw std::runtime_error("Invalid surface provided to CreateTextureFromSurface");
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(GAME_APP.GetRenderer(), surface);
    if (!texture)
    {
        throw std::runtime_error(std::format("Failed to create texture from surface: {}", SDL_GetError()));
    }

    return texture;
}