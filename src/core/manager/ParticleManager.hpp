#pragma once
/*
 *
 * 설명: 퍼즐 게임 스테이지별 배경 맵들을 관리하는 클래스
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

    using ParticleContainerList = std::list<std::shared_ptr<ParticleContainer>>;
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
    void AddParticleContainer(const std::shared_ptr<ParticleContainer>&& container);
    void AddParticleContainer(const std::shared_ptr<ParticleContainer>&& container, const SDL_FPoint& position);
    void RemoveParticleContainer(const ParticleContainer& container);

    void SetDrawEnabled(bool enabled) { is_draw_enabled_ = enabled; }
    [[nodiscard]] std::shared_ptr<ImageTexture> FindParticleTexture(const std::string& name);

private:    

    void ClearAllResources();

private:    

    ParticleContainerList containers_;
    TextureMap textures_;
    bool is_draw_enabled_{ true };
    bool is_initialized_{ false };
};