#include "GameBackground.hpp"

#include "../block/GroupBlock.hpp"
#include "../particles/BgParticleSystem.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../states/GameState.hpp"
#include "../../game/system/LocalPlayer.hpp"
#include "../../game/system/RemotePlayer.hpp"
#include "../../core/GameUtils.hpp"
#include "../../utils/Logger.hpp"
#include "../../utils/PathUtil.hpp"

#include <format>
#include <stdexcept>
#include <cmath>


GameBackground::GameBackground()
    : render_target_(nullptr, SDL_DestroyTexture)
{
    // ���� �÷��̾� �ʱ�ȭ
    PlayerData localData;
    const float deltaY = Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_Y - Constants::GroupBlock::NEXT_BLOCK_POS_Y;
    const float deltaX = Constants::GroupBlock::NEXT_BLOCK_POS_X - Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_X;
    const float next_angle = GameUtils::CalculateAngleInDegrees(deltaY, deltaX);
    GameUtils::SetDirectionVectorFromDegrees(next_angle, localData.directionVector.x, localData.directionVector.y);
    player_data_[Constants::PlayerType::Local] = localData;

    // ���� �÷��̾� �ʱ�ȭ
    PlayerData remoteData;
    const float playerDeltaY = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y - Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y;
    const float playerDeltaX = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X - Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_X;
    const float player_next_angle = GameUtils::CalculateAngleInDegrees(playerDeltaY, playerDeltaX);
    GameUtils::SetDirectionVectorFromDegrees(player_next_angle, remoteData.directionVector.x, remoteData.directionVector.y);
    player_data_[Constants::PlayerType::Remote] = remoteData;
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

void GameBackground::Update(float delta_time)
{
    delta_time = std::min<float>(delta_time, MAX_DELTA_TIME);

    // ��� �÷��̾� ������ ������Ʈ
    for (auto& [type, data] : player_data_) 
    {
        data.accumulatedTime += delta_time;

        while (data.groupBlocks.size() > MAX_BLOCKS_IN_QUEUE) 
        {
            data.groupBlocks.pop_front();
            LOGGER.Info("Excessive blocks detected for player {}, truncating queue", static_cast<int>(type));
        }

        if (data.isChangingBlock) 
        {
            UpdateBlockAnimations(delta_time, type);
        }
    }
}

GameBackground::BlockAnimationConfig GameBackground::GetAnimationConfig(Constants::PlayerType playerType) const
{
    BlockAnimationConfig config;

    if (playerType == Constants::PlayerType::Local) 
    {
        config.nextBlockPosX = Constants::GroupBlock::NEXT_BLOCK_POS_X;
        config.nextBlockPosY = Constants::GroupBlock::NEXT_BLOCK_POS_Y;
        config.nextBlockPosSmallX = Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_X;
        config.nextBlockPosSmallY = Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_Y;
    }
    else 
    {
        config.nextBlockPosX = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X;
        config.nextBlockPosY = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y;
        config.nextBlockPosSmallX = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_X;
        config.nextBlockPosSmallY = Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y;
    }

    config.blockVelocity = Constants::Background::NEW_BLOCK_VELOCITY;
    config.scaleVelocity = Constants::Background::NEW_BLOCK_SCALE_VELOCITY;
    config.directionVector = player_data_.at(playerType).directionVector;

    return config;
}

void GameBackground::UpdateBlockAnimations(float delta_time, Constants::PlayerType playerType)
{
    auto& data = player_data_[playerType];
    if (data.groupBlocks.size() < 3)
    {
        return;
    }

    auto& big_block = data.groupBlocks[0];
    auto& small_block = data.groupBlocks[1];
    auto& new_block = data.groupBlocks[2];

    auto config = GetAnimationConfig(playerType);

    bool can_erase = false;
    bool move_finished = false;

    // ū ��� ������Ʈ
    if (big_block) 
    {
        float y = big_block->GetY();
        y -= delta_time * config.blockVelocity;
        big_block->SetPosY(y);

        if (y <= -(Constants::Block::SIZE * 3)) 
        {
            can_erase = true;
        }
    }

    // ���� ��� ������Ʈ
    if (small_block) 
    {
        float x = small_block->GetX();
        float y = small_block->GetY();
        float width = small_block->GetWidth();
        float height = small_block->GetHeight();

        x += delta_time * config.directionVector.x * config.blockVelocity;
        y += delta_time * config.directionVector.y * config.blockVelocity;
        width += delta_time * config.scaleVelocity;
        height += delta_time * config.scaleVelocity;

        if (playerType == Constants::PlayerType::Local) 
        {
            if (x < config.nextBlockPosX) 
                x = config.nextBlockPosX;
        }
        else 
        {
            if (x > config.nextBlockPosX) 
                x = config.nextBlockPosX;
        }

        if (y < config.nextBlockPosY)
            y = config.nextBlockPosY;

        small_block->SetPosXY(x, y);

        if (width > Constants::Block::SIZE) 
            width = Constants::Block::SIZE;
        
        if (height > Constants::Block::SIZE) 
            height = Constants::Block::SIZE;

        small_block->SetScale(width, height);

        bool sameX = std::abs(x - config.nextBlockPosX) < 0.1f;
        bool sameY = std::abs(y - config.nextBlockPosY) < 0.1f;
        bool correctSize = std::abs(width - Constants::Block::SIZE) < 0.1f && std::abs(height - Constants::Block::SIZE) < 0.1f;

        if (sameX && sameY && correctSize)
        {
            small_block->SetPosXY(static_cast<float>(config.nextBlockPosX),static_cast<float>(config.nextBlockPosY));
            small_block->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
            move_finished = true;
        }
    }

    // �� ��� ������Ʈ
    if (new_block) 
    {
        float y = new_block->GetY();
        y -= delta_time * config.blockVelocity;

        if (static_cast<int>(y) <= config.nextBlockPosSmallY) 
        {
            

            y = static_cast<float>(config.nextBlockPosSmallY);

            /*if (playerType == Constants::PlayerType::Remote)
            {
                LOGGER.Info("GameBackground::UpdateBlockAnimations move_finish {} size {}", move_finished, data.groupBlocks.size());
            }*/
            if (move_finished) 
            {
                data.groupBlocks.pop_front();

                if (data.groupBlocks.size() != 2) 
                {
                    LOGGER.Info("GameBackground::UpdateBlockAnimations Invalid block count after pop: {} for player {}", data.groupBlocks.size(), static_cast<int>(playerType));

                    // ���� ���� - �׻� 2���� ��ϸ� ���⵵�� ��
                    while (data.groupBlocks.size() > 2) 
                    {
                        data.groupBlocks.pop_front();
                    }
                }

                data.isChangingBlock = false;

                // �÷��̾�� �˸�
                if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
                {
                    if (playerType == Constants::PlayerType::Local) 
                    {
                        if (auto& localPlayer = gameState->GetLocalPlayer()) 
                        {
                            localPlayer->PlayNextBlock();
                        }
                    }
                    else 
                    {
                        if (auto& remotePlayer = gameState->GetRemotePlayer()) 
                        {
                            remotePlayer->PlayNextBlock();
                        }
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

    // ��� ������
    for (size_t i = 0; i < background_textures_.size(); ++i) 
    {
        if (background_textures_[i]) 
        {
            background_textures_[i]->Render(i == 0 ? 0 : background_rects_[0].w, 0, &background_rects_[i]);
        }
    }

    // ��� ������
    bool shouldRenderBlocks = false;
    for (const auto& [type, data] : player_data_) 
    {
        if (!data.groupBlocks.empty()) {
            shouldRenderBlocks = true;
            break;
        }
    }

    if (shouldRenderBlocks && render_target_) 
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

        // ��� �÷��̾��� ��� ������
        for (const auto& [type, data] : player_data_) 
        {
            for (const auto& block : data.groupBlocks) 
            {
                if (block) block->Render();
            }
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

    for (auto& [type, data] : player_data_) 
    {
        data.groupBlocks.clear();
        data.isChangingBlock = false;
    }

    render_target_.reset();
}

void GameBackground::Reset()
{
    for (auto& [type, data] : player_data_) 
    {
        data.groupBlocks.clear();
        data.isChangingBlock = false;
    }
}

void GameBackground::SetNextBlock(const std::shared_ptr<GroupBlock>& block, Constants::PlayerType playerType)
{
    if (!block) 
    {
        return;
    }

    auto& data = player_data_[playerType];

    if (data.groupBlocks.size() >= MAX_BLOCKS_IN_QUEUE) 
    {
        LOGGER.Info("GameBackground::SetNextBlock Maximum block count reached for player {}, removing oldest block", static_cast<int>(playerType));
        data.groupBlocks.pop_front();
    }

    /*if (playerType == Constants::PlayerType::Remote)
    {
        LOGGER.Info("GameBackground::SetNextBlock size {}", data.groupBlocks.size());
    }*/


    data.groupBlocks.emplace_back(block);
    data.isChangingBlock = true;
    data.accumulatedTime = 0;
}

void GameBackground::SetChangingBlock(bool state, Constants::PlayerType playerType)
{
    player_data_[playerType].isChangingBlock = state;
}

bool GameBackground::IsChangingBlock(Constants::PlayerType playerType) const
{
    if (player_data_.count(playerType) == 0) return false;

    const auto& data = player_data_.at(playerType);
    return data.isChangingBlock && data.groupBlocks.size() == 3;
}

uint8_t GameBackground::GetNewBlockCount(Constants::PlayerType playerType) const
{
    if (player_data_.count(playerType) == 0)
    {
        return 0;
    }
    return static_cast<uint8_t>(player_data_.at(playerType).groupBlocks.size());
}

bool GameBackground::IsReadyGame() const
{
    return player_data_.count(Constants::PlayerType::Local) > 0 &&
        player_data_.count(Constants::PlayerType::Remote) > 0 &&
        player_data_.at(Constants::PlayerType::Local).groupBlocks.size() == 2 &&
        player_data_.at(Constants::PlayerType::Remote).groupBlocks.size() == 2;
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