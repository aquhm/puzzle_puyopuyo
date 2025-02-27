#pragma once
/**
 *
 * 설명: Game의 배경 표시 관련 class
 *
 */

#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../particles/BgParticleSystem.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <deque>
#include <string>
#include <memory>

class ImgTexture;
class GroupBlock;

enum class BackgroundType : uint8_t
{
    GrassLand = 0,
    IceLand = 13
};

class GameBackground : public RenderableObject 
{
public:
    GameBackground();
    ~GameBackground() override = default;

    GameBackground(const GameBackground&) = delete;
    GameBackground& operator=(const GameBackground&) = delete;
    GameBackground(GameBackground&&) = delete;
    GameBackground& operator=(GameBackground&&) = delete;

    [[nodiscard]] virtual bool Initialize();
    void Update(float deltaTime) override;
    [[nodiscard]] virtual void Render() override;
    void Release() override;

    void Reset();
    void SetNextBlock(std::shared_ptr<GroupBlock> block);
    void SetPlayerNextBlock(std::shared_ptr<GroupBlock> block);

    [[nodiscard]] bool IsChangingBlock() const;
    [[nodiscard]] bool IsChangingPlayerBlock() const;
    [[nodiscard]] bool IsReadyGame() const;
    [[nodiscard]] uint8_t GetNewBlockCount() const { return static_cast<uint8_t>(group_blocks_.size()); }
    [[nodiscard]] uint8_t GetMapIndex() const { return map_index_; }

    // Setters 모음
    void SetChangingBlock(bool state) { is_changing_block_ = state; }
    void SetChangingPlayerBlock(bool state) { is_player_changing_block_ = state; }

 protected:
     [[nodiscard]] virtual bool LoadBackgroundTextures();
     [[nodiscard]] bool CreateRenderTarget();
     void UpdateBlockAnimations(float deltaTime);
     void UpdatePlayerBlockAnimations(float deltaTime);

protected:


    std::array<std::shared_ptr<ImageTexture>, 2> background_textures_;
    std::array<std::shared_ptr<ImageTexture>, 2> block_preview_textures_;
    std::array<SDL_FRect, 2> background_rects_;

    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> render_target_;
    SDL_FRect render_target_rect_{};

    bool is_changing_block_{ false };
    bool is_player_changing_block_{ false };
    std::deque<std::shared_ptr<GroupBlock>> group_blocks_;
    std::deque<std::shared_ptr<GroupBlock>> player_group_blocks_;

    SDL_FPoint direction_vector_;
    SDL_FPoint player_direction_vector_;

    uint8_t map_index_{};
    float accumulated_time_{ 0.0f };
    bool is_initialized_{ false };    
};

