#pragma once

#include "../../core/IManager.hpp"
#include "../../utils/Math.hpp"
#include <memory>
#include <string>
#include <list>
#include <unordered_map>

class ImgTexture;

namespace effects {

    class ParticleContainer;

    class ParticleManager final : public IManager {
    public:
        // Delete copy/move operations
        ParticleManager(const ParticleManager&) = delete;
        ParticleManager& operator=(const ParticleManager&) = delete;
        ParticleManager(ParticleManager&&) = delete;
        ParticleManager& operator=(ParticleManager&&) = delete;

        // Singleton access
        static ParticleManager& GetInstance();

        // IManager interface implementation
        [[nodiscard]] bool Initialize() override;
        void Update(float deltaTime) override;
        void Release() override;
        [[nodiscard]] std::string_view GetName() const override { return "ParticleManager"; }

        // Container management
        void AddContainer(std::unique_ptr<ParticleContainer> container);
        void AddContainer(std::unique_ptr<ParticleContainer> container, const Vector2f& position);
        void RemoveContainer(const ParticleContainer& container);

        // Rendering
        void Render();
        void Render(uint8_t playerId);
        void SetVisible(bool visible) { is_visible_ = visible; }
        [[nodiscard]] bool IsVisible() const { return is_visible_; }

        // Resource management
        [[nodiscard]] std::shared_ptr<ImgTexture> GetTexture(const std::string& name) const;
        void AddTexture(const std::string& name, std::shared_ptr<ImgTexture> texture);

    private:
        ParticleManager() = default;
        ~ParticleManager() override = default;

        using ContainerList = std::list<std::unique_ptr<ParticleContainer>>;
        using TextureMap = std::unordered_map<std::string, std::shared_ptr<ImgTexture>>;

        ContainerList containers_;
        TextureMap textures_;
        bool is_visible_{ true };

        void CleanupInactiveContainers();
    };

#define PARTICLE_MANAGER effects::ParticleManager::GetInstance()

} // namespace effects