#pragma once

#include "BackgroundTypes.hpp"
#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../particles/ParticleSystem.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <deque>
#include <string>
#include <memory>

class ImgTexture;
class GroupBlock;

namespace bg 
{
    class GameBackground : public RenderableObject 
    {
    public:
        GameBackground();
        ~GameBackground() override;

        // Delete copy/move operations
        GameBackground(const GameBackground&) = delete;
        GameBackground& operator=(const GameBackground&) = delete;
        GameBackground(GameBackground&&) = delete;
        GameBackground& operator=(GameBackground&&) = delete;

        // Core functionality
        [[nodiscard]] virtual bool Initialize();
        void Update(float deltaTime) override;
        void Render() override;
        void Release() override;

        // Block management
        void Reset();
        void SetNextBlock(std::shared_ptr<GroupBlock> block);
        void SetPlayerNextBlock(std::shared_ptr<GroupBlock> block);

        // State queries
        [[nodiscard]] bool IsChangingBlock() const;
        [[nodiscard]] bool IsChangingPlayerBlock() const;
        [[nodiscard]] bool IsReadyGame() const;
        [[nodiscard]] uint8_t GetNewBlockCount() const { return static_cast<uint8_t>(group_blocks_.size()); }
        [[nodiscard]] int16_t GetMapIndex() const { return map_index_; }

        // State setters
        void SetChangingBlock(bool state) { is_changing_block_ = state; }
        void SetChangingPlayerBlock(bool state) { is_player_changing_block_ = state; }

    protected:
        struct Vector2f 
        {
            float x{ 0.0f };
            float y{ 0.0f };
        };

        // Background resources
        std::array<std::unique_ptr<ImageTexture>, 2> background_textures_;
        std::array<std::unique_ptr<ImageTexture>, 2> block_preview_textures_;
        std::array<SDL_Rect, 2> background_rects_;

        // Render target
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> render_target_;
        SDL_Rect render_target_rect_{};

        // Block management
        bool is_changing_block_{ false };
        bool is_player_changing_block_{ false };
        std::deque<std::shared_ptr<GroupBlock>> group_blocks_;
        std::deque<std::shared_ptr<GroupBlock>> player_group_blocks_;

        // Movement vectors
        Vector2f direction_vector_;
        Vector2f player_direction_vector_;

        // Map properties
        int16_t map_index_{ -1 };
        std::string file_path_{ "./Image/BG/BG" };
        float accumulated_time_{ 0.0f };
        bool is_initialized_{ false };

        // Helper functions
        [[nodiscard]] virtual bool LoadBackgroundTextures();
        [[nodiscard]] bool CreateRenderTarget();
        void UpdateBlockAnimations(float deltaTime);
    };

} // namespace bg