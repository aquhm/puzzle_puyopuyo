#pragma once
/*
 *
 * 설명: IResource를 구현한 각종 asset 관리 Class
 *
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <filesystem>
#include <concepts>

#include <SDL3/SDL_filesystem.h>

#include "IManager.hpp"
#include "../IResource.hpp"
#include "../../resource/ResourcePathTrait.hpp"
#include "../../utils/Logger.hpp"


struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;


class ResourceManager final : public IManager 
{
public:

    using ResourceContainer = std::unordered_map<std::string, std::shared_ptr<IResource>>;

    ResourceManager() = default;
    ~ResourceManager() override = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    
    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    [[nodiscard]] std::string_view GetName() const override { return "ResourceManager"; }

    SDL_Texture* CreateTextureFromSurface(SDL_Surface* surface) const;

    template<std::derived_from<IResource> T>
    [[nodiscard]] std::shared_ptr<T> LoadResource(const std::string& path);

    template<std::derived_from<IResource> T>
    [[nodiscard]] std::shared_ptr<T> GetResource(const std::string& path);

    template<std::derived_from<IResource> T>
    void UnloadResource(const std::string& path);

private:
    bool IsResourceLoaded(const std::string& filename) const;

    template<std::derived_from<IResource> T>
    [[nodiscard]] ResourceContainer& GetResourceContainer();

    template<std::derived_from<IResource> T>
    [[nodiscard]] const ResourceContainer& GetResourceContainer() const;

private:
    
    std::unordered_map<std::type_index, ResourceContainer> resources_;

    SDL_Renderer* renderer_{ nullptr };
   
};


template<std::derived_from<IResource> T>
std::shared_ptr<T> ResourceManager::GetResource(const std::string& path)
{
    std::string exeBasePath = PathUtil::GetExecutableBasePath();
    if (exeBasePath.empty()) 
    {    
        SDL_Log("Executable path is empty()");
        return nullptr;
    }

    std::string assetPath = PathUtil::CombinePaths(ResourcePathTrait<T>::BaseDir, path);
    std::string fullPath = PathUtil::CombinePaths(exeBasePath, assetPath);

    const auto& container = GetResourceContainer<T>();

    if (auto it = container.find(fullPath); it != container.end())
    {
        return static_pointer_cast<T>(it->second);
    }
    
    try
    {
        return LoadResource<T>(fullPath);
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load resource '%s': %s", fullPath.c_str(), e.what());
        return nullptr;
    }
}


template<std::derived_from<IResource> T>
void ResourceManager::UnloadResource(const std::string& path)
{
    auto& container = GetResourceContainer<T>();
    container.erase(path);
}


template<std::derived_from<IResource> T>
ResourceManager::ResourceContainer& ResourceManager::GetResourceContainer()
{
    return resources_[std::type_index(typeid(T))];
}

template<std::derived_from<IResource> T>
const ResourceManager::ResourceContainer& ResourceManager::GetResourceContainer() const
{
    if (auto it = resources_.find(std::type_index(typeid(T))); it != resources_.end())
    {
        return it->second;
    }

    static const ResourceContainer empty;
    return empty;

}

template<std::derived_from<IResource> T>
std::shared_ptr<T> ResourceManager::LoadResource(const std::string& path)
{
    auto& container = GetResourceContainer<T>();

    // 이미 로드된 리소스인지 확인
    if (auto it = container.find(path); it != container.end())
    {
        return static_pointer_cast<T>(it->second);
    }

    // 새 리소스 생성 및 로드
    auto resource = std::make_shared<T>();

    if (!resource->Load(path))
    {
        throw std::runtime_error("Failed to load resource: " + path);
    }

    auto [it, inserted] = container.emplace(path, resource);
    return static_pointer_cast<T>(it->second);
}