#pragma once
/*
 *
 * ����: ���� ���� ���������� ��� �ʵ��� �����ϴ� Ŭ����
 *
 */


#include "IManager.hpp"
#include "../IRenderable.hpp"
#include "../../game/effect/ParticleContainer.hpp"
#include <list>
#include <string>
#include <unordered_map>
#include <memory>
#include <SDL3/SDL.h> 

class ImageTexture;

class ParticleManager : public IManager, public IRenderable 
{
public:

    using ParticleContainerList = std::list<std::unique_ptr<ParticleContainer>>;
    using TextureMap = std::unordered_map<std::string, std::shared_ptr<ImageTexture>>;

    ParticleManager() = default;
    ~ParticleManager() override;
   

    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;
    ParticleManager(ParticleManager&&) = delete;
    ParticleManager& operator=(ParticleManager&&) = delete;

    [[nodiscard]] bool Initialize() override;
    [[nodiscard]] std::string_view GetName() const override { return "ParticleManager"; }
    void Update(float deltaTime) override;
    void Release() override;
    void Render() override;
    [[nodiscard]] int GetRenderPriority() const override { return 100; }

    
    void RenderForPlayer(uint8_t playerId);
    void AddParticleContainer(std::unique_ptr<ParticleContainer> container);
    void AddParticleContainer(std::unique_ptr<ParticleContainer> container, const SDL_FPoint& position);
    void RemoveParticleContainer(const ParticleContainer& container);

    void SetDrawEnabled(bool enabled) { isDrawEnabled_ = enabled; }
    [[nodiscard]] std::shared_ptr<ImageTexture> FindParticleTexture(const std::string& name);

private:    

    void ClearAllResources();

private:    

    ParticleContainerList containers_;
    TextureMap textures_;
    bool isDrawEnabled_{ true };
    bool isInitialized_{ false };
};