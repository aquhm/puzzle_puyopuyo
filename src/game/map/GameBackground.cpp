#include "GameBackground.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../groupblock/GroupBlock.hpp"
#include <format>
#include <stdexcept>
#include <cmath>

namespace bg {

    GameBackground::GameBackground()
        : render_target_(nullptr, SDL_DestroyTexture)
    {
        const float next_angle = std::atan2(
            NEXT_BLOCK_POS_SMALL_Y - NEXT_BLOCK_POS_Y,
            NEXT_BLOCK_POS_X - NEXT_BLOCK_POS_SMALL_X) * 180.0f / static_cast<float>(M_PI);

        direction_vector_.x = std::cos(next_angle * static_cast<float>(M_PI) / 180.0f);
        direction_vector_.y = -std::sin(next_angle * static_cast<float>(M_PI) / 180.0f);

        const float player_next_angle = std::atan2(
            NEXT_PLAYER_BLOCK_POS_SMALL_Y - NEXT_PLAYER_BLOCK_POS_Y,
            NEXT_PLAYER_BLOCK_POS_X - NEXT_PLAYER_BLOCK_POS_SMALL_X) * 180.0f / static_cast<float>(M_PI);

        player_direction_vector_.x = std::cos(player_next_angle * static_cast<float>(M_PI) / 180.0f);
        player_direction_vector_.y = -std::sin(player_next_angle * static_cast<float>(M_PI) / 180.0f);
    }

    GameBackground::~GameBackground() = default;

    bool GameBackground::Initialize() 
    {
        if (is_initialized_) 
        {
            return true;
        }

        try 
        {
            if (!LoadBackgroundTextures() || !CreateRenderTarget()) 
            {
                return false;
            }

            is_initialized_ = true;
            return true;
        }
        catch (const std::exception& e) 
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Background initialization failed: %s", e.what());
            return false;
        }
    }

    bool GameBackground::LoadBackgroundTextures() 
    {
        for (int i = 0; i < 2; ++i) 
        {
            background_textures_[i] = std::make_unique<ImgTexture>();
            block_preview_textures_[i] = std::make_unique<ImgTexture>();

            if (!background_textures_[i] || !block_preview_textures_[i]) 
            {
                throw std::runtime_error("Failed to create texture objects");
            }

            auto bg_filename = std::format("{}{:02d}/bg{:02d}_{:02d}.png", file_path_, map_index_, map_index_, i);

            if (!background_textures_[i]->LoadTexture(bg_filename)) 
            {
                throw std::runtime_error("Failed to load background texture");
            }

            auto mask_filename = std::format("{}{:02d}/bg{:02d}_mask{}.png", file_path_, map_index_, map_index_, i == 0 ? "" : "_2");

            if (!block_preview_textures_[i]->LoadTexture(mask_filename)) 
            {
                throw std::runtime_error("Failed to load mask texture");
            }
        }

        // Setup background rectangles
        background_rects_[0] = {
            0, 0,
            background_textures_[0]->GetWidth(),
            GAMEAPP.GetWindowHeight()
        };

        background_rects_[1] = {
            0, 0,
            background_textures_[1]->GetWidth(),
            background_rects_[0].h
        };

        return true;
    }

    bool GameBackground::CreateRenderTarget() {
        render_target_.reset(SDL_CreateTexture(
            GAMEAPP.GetRenderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            BackgroundConstants::MASK_WIDTH,
            BackgroundConstants::MASK_HEIGHT
        ));

        if (!render_target_) {
            throw std::runtime_error(std::format(
                "Failed to create render target: {}", SDL_GetError()));
        }

        SDL_SetTextureBlendMode(render_target_.get(), SDL_BLENDMODE_BLEND);

        render_target_rect_ = {
            BackgroundConstants::MASK_POSITION_X,
            BackgroundConstants::MASK_POSITION_Y,
            BackgroundConstants::MASK_WIDTH,
            BackgroundConstants::MASK_HEIGHT
        };

        return true;
    }

    void GameBackground::Update(float delta_time) {
        accumulated_time_ += delta_time;

        if (is_changing_block_) {
            UpdateBlockAnimations(delta_time);
        }
    }

    void GameBackground::UpdateBlockAnimations(float delta_time) {
        if (group_blocks_.size() != 3) return;

        auto& big_block = group_blocks_[0];
        auto& small_block = group_blocks_[1];
        auto& new_block = group_blocks_[2];

        bool can_erase = false;
        bool move_finished = false;

        // Update big block
        if (big_block) {
            float y = big_block->GetY();
            y -= delta_time * BackgroundConstants::NEW_BLOCK_VELOCITY;
            big_block->SetY(y);

            if (y < -(BLOCK_SIZE * 3)) {
                can_erase = true;
            }
        }

        // Update small block
        if (small_block) {
            float x = small_block->GetX();
            float y = small_block->GetY();
            float width = small_block->GetWidth();
            float height = small_block->GetHeight();

            x += delta_time * direction_vector_.x * BackgroundConstants::NEW_BLOCK_VELOCITY;
            y += delta_time * direction_vector_.y * BackgroundConstants::NEW_BLOCK_VELOCITY;
            width += delta_time * BackgroundConstants::NEW_BLOCK_SCALE_VELOCITY;
            height += delta_time * BackgroundConstants::NEW_BLOCK_SCALE_VELOCITY;

            x = std::max(x, static_cast<float>(NEXT_BLOCK_POS_X));
            y = std::max(y, static_cast<float>(NEXT_BLOCK_POS_Y));
            width = std::min(width, static_cast<float>(BLOCK_SIZE));
            height = std::min(height, static_cast<float>(BLOCK_SIZE));

            small_block->SetPosition(x, y);
            small_block->SetSize(width, height);

            if (x == NEXT_BLOCK_POS_X && y == NEXT_BLOCK_POS_Y &&
                width == BLOCK_SIZE && height == BLOCK_SIZE) {
                move_finished = true;
            }
        }

        // Update new block
        if (new_block) {
            float y = new_block->GetY();
            y -= delta_time * BackgroundConstants::NEW_BLOCK_VELOCITY;

            if (y < NEXT_BLOCK_POS_SMALL_Y) {
                y = NEXT_BLOCK_POS_SMALL_Y;

                if (can_erase && move_finished) {
                    group_blocks_.pop_front();
                    is_changing_block_ = false;
                }
            }
            new_block->SetY(y);
        }
    }

    void GameBackground::Render() {
        if (!is_visible_) return;

        // Render background layers
        for (size_t i = 0; i < background_textures_.size(); ++i) {
            if (background_textures_[i]) {
                background_textures_[i]->Render(
                    i == 0 ? 0 : background_rects_[0].w,
                    0,
                    &background_rects_[i]
                );
            }
        }

        // Render block preview
        if (!group_blocks_.empty() && !player_group_blocks_.empty() && render_target_) {
            SDL_SetRenderTarget(GAMEAPP.GetRenderer(), render_target_.get());
            SDL_SetRenderDrawColor(GAMEAPP.GetRenderer(), 0, 0, 0, 0);
            SDL_RenderClear(GAMEAPP.GetRenderer());

            // Render mask background
            if (block_preview_textures_[0]) {
                block_preview_textures_[0]->Render(0, 0);
                block_preview_textures_[0]->Render(32, 0, nullptr, 0, SDL_FLIP_HORIZONTAL);
            }

            // Render blocks
            for (const auto& block : group_blocks_