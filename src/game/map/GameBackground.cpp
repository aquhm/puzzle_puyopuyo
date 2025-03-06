#include "GameBackground.hpp"

#include "../block/GroupBlock.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../states/GameState.hpp"
#include "../../game/system/LocalPlayer.hpp"
#include "../../game/system/RemotePlayer.hpp"
#include "../particles/BgParticleSystem.hpp"
#include "../../core/GameUtils.hpp"
#include "../../utils/Logger.hpp"


#include <format>
#include <stdexcept>
#include <cmath>
#include "../../utils/PathUtil.hpp"


GameBackground::GameBackground()
    : render_target_(nullptr, SDL_DestroyTexture)
{
    const float deltaY = Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_Y - Constants::GroupBlock::NEXT_BLOCK_POS_Y;
    const float deltaX = Constants::GroupBlock::NEXT_BLOCK_POS_X - Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_X;
    const float next_angle = GameUtils::CalculateAngleInDegrees(deltaY, deltaX);
    GameUtils::SetDirectionVectorFromDegrees(next_angle, direction_vector_.x, direction_vector_.y);

    const float playerDeltaY = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y - Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y;
    const float playerDeltaX = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X - Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_X;
    const float player_next_angle = GameUtils::CalculateAngleInDegrees(playerDeltaY, playerDeltaX);
    GameUtils::SetDirectionVectorFromDegrees(player_next_angle, player_direction_vector_.x, player_direction_vector_.y);
}

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
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Background initialization failed: %s", e.what());
        return false;
    }
}

bool GameBackground::LoadBackgroundTextures()
{
    try
    {
        std::string bgPath = PathUtil::GetBgPath();

        for (int i = 0; i < 2; ++i)
        {
            auto bg_filename = std::format("{}/bg{:02d}/bg{:02d}_{:02d}.png", bgPath, map_index_, map_index_, i);
            auto mask_filename = std::format("{}/bg{:02d}/bg{:02d}_mask{}.png", bgPath, map_index_, map_index_, i == 0 ? "" : "_2");

            background_textures_[i] = ImageTexture::Create(bg_filename);
            mask_textures_[i] = ImageTexture::Create(mask_filename);

            if (!background_textures_[i] || !mask_textures_[i])
            {
                throw std::runtime_error("Failed to load background or mask texture");
            }
        }

        background_rects_[0] = 
        {
            0, 0,
            background_textures_[0]->GetWidth(),
            static_cast<float>(GAME_APP.GetWindowHeight())
        };

        background_rects_[1] = 
        {
            0, 0,
            static_cast<float>(background_textures_[1]->GetWidth()),
            background_rects_[0].h
        };

        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load background textures: %s", e.what());
        return false;
    }
}

bool GameBackground::CreateRenderTarget() 
{
    render_target_.reset(SDL_CreateTexture(
        GAME_APP.GetRenderer(),
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        Constants::Background::MASK_WIDTH,
        Constants::Background::MASK_HEIGHT
    ));

    if (!render_target_) 
    {
        throw std::runtime_error(std::format("Failed to create render target: {}", SDL_GetError()));
    }

    SDL_SetTextureBlendMode(render_target_.get(), SDL_BLENDMODE_BLEND);

    render_target_rect_ = 
    {
        Constants::Background::MASK_POSITION_X,
        Constants::Background::MASK_POSITION_Y,
        Constants::Background::MASK_WIDTH,
        Constants::Background::MASK_HEIGHT
    };

    return true;
}

void GameBackground::Update(float delta_time)
{
    accumulated_time_ += delta_time;

    if (is_changing_block_)
    {
        UpdateBlockAnimations(delta_time);
    }

    if (is_player_changing_block_)
    {
        UpdatePlayerBlockAnimations(delta_time);
    }
}

void GameBackground::UpdateBlockAnimations(float delta_time)
{
    if (group_blocks_.size() != 3)
    {
        return;
    }

    auto& big_block = group_blocks_[0];
    auto& small_block = group_blocks_[1];
    auto& new_block = group_blocks_[2];

    bool can_erase = false;
    bool move_finished = false;

    if (big_block)
    {
        float y = big_block->GetY();
        y -= delta_time * Constants::Background::NEW_BLOCK_VELOCITY;
        big_block->SetPosY(y);

        if (y < -(Constants::Block::SIZE * 3))
        {
            can_erase = true;
        }
    }

    if (small_block)
    {
        float x = small_block->GetX();
        float y = small_block->GetY();

        float width = small_block->GetWidth();
        float height = small_block->GetHeight();

        x += delta_time * direction_vector_.x * Constants::Background::NEW_BLOCK_VELOCITY;
        y += delta_time * direction_vector_.y * Constants::Background::NEW_BLOCK_VELOCITY;

        width += delta_time * Constants::Background::NEW_BLOCK_SCALE_VELOCITY;
        height += delta_time * Constants::Background::NEW_BLOCK_SCALE_VELOCITY;

        x = std::max<float>(x, static_cast<float>(Constants::GroupBlock::NEXT_BLOCK_POS_X));
        y = std::max<float>(y, static_cast<float>(Constants::GroupBlock::NEXT_BLOCK_POS_Y));
        width = std::min<float>(width, static_cast<float>(Constants::Block::SIZE));
        height = std::min<float>(height, static_cast<float>(Constants::Block::SIZE));

        small_block->SetPosXY(x, y);
        small_block->SetScale(width, height);

        if (static_cast<int>(x) == Constants::GroupBlock::NEXT_BLOCK_POS_X && 
            static_cast<int>(y) == Constants::GroupBlock::NEXT_BLOCK_POS_Y &&
            width == Constants::Block::SIZE && height == Constants::Block::SIZE)
        {
            move_finished = true;
        }
    }

    if (new_block)
    {
        float y = new_block->GetY();
        y -= delta_time * Constants::Background::NEW_BLOCK_VELOCITY;

        if (static_cast<int>(y) < Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_Y)
        {
            y = static_cast<float>(Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_Y);

            if (can_erase && move_finished)
            {
                group_blocks_.pop_front();
                is_changing_block_ = false;

                if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
                {
                    if (auto localPlayer = gameState->GetLocalPlayer())
                    {
                        localPlayer->PlayNextBlock();
                    }
                }
            }
        }
        new_block->SetPosY(y);
    }
}

void GameBackground::UpdatePlayerBlockAnimations(float delta_time)
{
    if (player_group_blocks_.size() != 3)
    {
        return;
    }

    auto& big_block = player_group_blocks_[0];
    auto& small_block = player_group_blocks_[1];
    auto& new_block = player_group_blocks_[2];

    bool can_erase = false;
    bool move_finished = false;

    if (big_block)
    {
        float y = big_block->GetY();
        y -= delta_time * Constants::Background::NEW_BLOCK_VELOCITY;
        big_block->SetPosY(y);

        if (y < -(Constants::Block::SIZE * 3))
        {
            can_erase = true;
        }
    }

    if (small_block)
    {        
        float x = small_block->GetX();
        float y = small_block->GetY();

        float width = small_block->GetWidth();
        float height = small_block->GetHeight();

        x += delta_time * player_direction_vector_.x * Constants::Background::NEW_BLOCK_VELOCITY;
        y += delta_time * player_direction_vector_.y * Constants::Background::NEW_BLOCK_VELOCITY;

        width += delta_time * Constants::Background::NEW_BLOCK_SCALE_VELOCITY;
        height += delta_time * Constants::Background::NEW_BLOCK_SCALE_VELOCITY;

        x = std::min<float>(x, static_cast<float>(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X));
        y = std::max<float>(y, static_cast<float>(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y));
        width = std::min<float>(width, static_cast<float>(Constants::Block::SIZE));
        height = std::min<float>(height, static_cast<float>(Constants::Block::SIZE));

        small_block->SetPosXY(x, y);
        small_block->SetScale(width, height);

        if (static_cast<int>(x) == Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X &&
            static_cast<int>(y) == Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y &&
            width == Constants::Block::SIZE && height == Constants::Block::SIZE)
        {
            move_finished = true;
        }
    }

    if (new_block)
    {
        float y = new_block->GetY();
        y -= delta_time * Constants::Background::NEW_BLOCK_VELOCITY;

        if (static_cast<int>(y) < Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y)
        {
            y = static_cast<float>(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y);

            if (can_erase && move_finished)
            {
                player_group_blocks_.pop_front();
                is_player_changing_block_ = false;

                if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
                {
                    if (auto remotePlayer = gameState->GetRemotePlayer())
                    {
                        remotePlayer->PlayNextBlock();
                    }
                }
            }
        }
        new_block->SetPosY(y);
    }
}

void GameBackground::Render() 
{
    if (!is_visible_)
    {
        return;
    }

    for (size_t i = 0; i < background_textures_.size(); ++i) 
    {
        if (background_textures_[i]) 
        {
            background_textures_[i]->Render(i == 0 ? 0 : background_rects_[0].w, 0, &background_rects_[i]);
        }
    }

    if (!group_blocks_.empty() && !player_group_blocks_.empty() && render_target_) 
    {
        auto renderer = GAME_APP.GetRenderer();
        SDL_SetRenderTarget(renderer, render_target_.get());
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0);
        SDL_RenderClear(renderer);

        if (mask_textures_[0])
        {
            mask_textures_[0]->Render(0, 0);
            mask_textures_[0]->Render(32, 0, nullptr, 0.0f, nullptr, SDL_FLIP_HORIZONTAL);
        }

        for (const auto& block : group_blocks_) 
        {
            if (block) block->Render();
        }

        for (const auto& block : player_group_blocks_) 
        {
            if (block) block->Render();
        }

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, render_target_.get(), nullptr, &render_target_rect_);
    }

    if (mask_textures_[1])
    {
        mask_textures_[1]->Render(Constants::Background::MASK_POSITION_X, Constants::Background::MASK_POSITION_Y);
    }
}

void GameBackground::Release() 
{
    background_textures_[0].reset();
    background_textures_[1].reset();
    mask_textures_[0].reset();
    mask_textures_[1].reset();

    group_blocks_.clear();
    player_group_blocks_.clear();
    render_target_.reset();
}

void GameBackground::Reset() 
{
    group_blocks_.clear();
    player_group_blocks_.clear();
    is_changing_block_ = false;
    is_player_changing_block_ = false;
}

void GameBackground::SetNextBlock(const std::shared_ptr<GroupBlock>& block) 
{
    if (!block)
    {
        return;
    }

    group_blocks_.emplace_back(block);
    is_changing_block_ = true;
}

void GameBackground::SetPlayerNextBlock(const std::shared_ptr<GroupBlock>& block)
{
    if (!block)
    {
        return;
    }

    player_group_blocks_.emplace_back(block);
    is_player_changing_block_ = true;
}

bool GameBackground::IsChangingBlock() const 
{
    return is_changing_block_ && group_blocks_.size() == 3;
}

bool GameBackground::IsChangingPlayerBlock() const 
{
    return is_player_changing_block_ && player_group_blocks_.size() == 3;
}

bool GameBackground::IsReadyGame() const 
{
    return group_blocks_.size() == 2 && player_group_blocks_.size() == 2;
}