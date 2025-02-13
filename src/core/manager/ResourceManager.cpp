#include "ResourceManager.hpp"
//#include "ImageTexture.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <format>

bool ResourceManager::Initialize()
{
    try
    {
        resources_.clear();
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ResourceManager initialization failed: %s", e.what());
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
    // 모든 리소스 컨테이너를 검사
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

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (!texture)
    {
        throw std::runtime_error(
            std::format("Failed to create texture from surface: {}", SDL_GetError())
        );
    }

    return texture;
}

template<typename T> requires std::is_base_of_v<IResource, T>
T* ResourceManager::LoadResource(const std::string& path)
{
    auto& container = GetResourceContainer<T>();

    // 이미 로드된 리소스인지 확인
    if (auto it = container.find(path); it != container.end())
    {
        return static_cast<T*>(it->second.get());
    }

    // 새 리소스 생성 및 로드
    auto resource = std::make_unique<T>();
    T* resourcePtr = resource.get();

    if (!resource->Load(path))
    {
        throw std::runtime_error("Failed to load resource: " + path);
    }

    container.emplace(path, std::move(resource));
    return resourcePtr;
}

template<typename T> requires std::is_base_of_v<IResource, T>
T* ResourceManager::GetResource(const std::string& path)
{
    const auto& container = GetResourceContainer<T>();

    if (auto it = container.find(path); it != container.end())
    {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T> requires std::is_base_of_v<IResource, T>
void ResourceManager::UnloadResource(const std::string& path)
{
    auto& container = GetResourceContainer<T>();
    container.erase(path);
}

template<typename T>
ResourceManager::ResourceContainer& ResourceManager::GetResourceContainer()
{
    return resources_[std::type_index(typeid(T))];
}

template<typename T>
const ResourceManager::ResourceContainer& ResourceManager::GetResourceContainer() const
{
    if (auto it = resources_.find(std::type_index(typeid(T))); it == resources_.end())
    {
        static const ResourceContainer empty;
        return empty;
    }
    return it->second;
}

// Helper function to create error message
namespace detail {
    template<typename T>
    std::string GetResourceTypeName() {
        return typeid(T).name();
    }

    template<>
    std::string GetResourceTypeName<ImageTexture>() {
        return "ImageTexture";
    }
}