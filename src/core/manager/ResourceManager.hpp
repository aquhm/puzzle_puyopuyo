#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <filesystem>

#include "IManager.hpp"
#include "../IResource.hpp"


// Forward declarations
struct SDL_Renderer;
class ImgTexture;

class ResourceManager final : public IManager 
{
public:

    // Delete copy/move
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    // IManager interface
    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    [[nodiscard]] std::string_view GetName() const override { return "ResourceManager"; }


    // Resource management
    template<typename T> requires std::is_base_of_v<IResource, T>
    [[nodiscard]] T* LoadResource(const std::string& path);

    template<typename T> requires std::is_base_of_v<IResource, T>
    [[nodiscard]] T* GetResource(const std::string& path);

    template<typename T> requires std::is_base_of_v<IResource, T>
    void UnloadResource(const std::string& path);

    // SDL specific
    SDL_Texture* CreateTextureFromSurface(SDL_Surface* surface) const;
  

private:
    ResourceManager() = default;
    ~ResourceManager() override = default;

    bool IsResourceLoaded(const std::string& filename) const;

    // Resource container type (type_index를 키로 사용하여 리소스 타입별로 관리)
    using ResourceContainer = std::unordered_map<std::string, std::unique_ptr<IResource>>;
    std::unordered_map<std::type_index, ResourceContainer> resources_;

    SDL_Renderer* renderer_{ nullptr };

    template<typename T>
    [[nodiscard]] ResourceContainer& GetResourceContainer();

    template<typename T>
    [[nodiscard]] const ResourceContainer& GetResourceContainer() const;
};