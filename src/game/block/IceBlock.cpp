#include "IceBlock.hpp"
#include "../../core/GameApp.hpp"
#include "../../states/GameState.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../game/system/GamePlayer.hpp"
#include "../../utils/RectUtil.hpp"


void IceBlock::SetState(BlockState state) 
{
    state_ = state;

    switch (state_) 
    {
    case BlockState::Stationary:
        sourceRect_.x = blockOriginPos_.x;
        sourceRect_.y = blockOriginPos_.y;
        downVelocity_ = 0.0f;
        accumEffectTime_ = 0.0f;
        break;

    case BlockState::Destroying:
        alpha_ = 255.0f;
        accumEffectTime_ = 0.0f;
        break;

    case BlockState::DownMoving:
        break;

    case BlockState::PlayOut:
        accumEffectTime_ = 0.0f;
        rotationAngle_ = 0.0f;
        break;

    default:
        break;
    }
}

void IceBlock::Update(float deltaTime) 
{
    switch (state_) 
    {
    case BlockState::Destroying:
        UpdateDestroying(deltaTime);
        break;

    case BlockState::DownMoving:
        UpdateDownMoving(deltaTime);
        break;

    default:
        break;
    }
}

void IceBlock::UpdateDestroying(float deltaTime) 
{
    constexpr float DESTROY_DURATION = 1.5f;
    constexpr float ALPHA_RATE = 255.0f / DESTROY_DURATION;

    accumEffectTime_ += deltaTime;

    if (accumEffectTime_ <= DESTROY_DURATION) 
    {
        if (texture_ && alpha_ > 0) 
        {
            alpha_ = 255.0f - (accumEffectTime_ * ALPHA_RATE);
        }
    }
}

void IceBlock::UpdateDownMoving(float deltaTime) 
{
    float fallSpeed = deltaTime * static_cast<float>(Constants::Board::BOARD_Y_COUNT - indexY_);

    downVelocity_ += fallSpeed * 0.1f;
    position_.y += downVelocity_;
    SetY(position_.y);

    Block* (*blocks)[Constants::Board::BOARD_X_COUNT] = nullptr;

    if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
    {
        blocks = gameState->GetGameBlocks(playerID_);
    }

    if (!blocks)
    {
        return;
    }

    bool hasCollision = false;
    bool canMove = true;

    SDL_Rect targetRect, controlRect, resultRect;

    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; ++y) 
    {
        Block* block = blocks[y][indexX_];
        if (!block || block == this)
        {
            continue;
        }

        if (block->GetState() == BlockState::Stationary) 
        {
            RectUtils::ConvertFRectToRect(destination_rect_, &controlRect);
            RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

            if (SDL_GetRectIntersection(&controlRect, &targetRect, &resultRect))
            {
                SetY(resultRect.y - Constants::Block::SIZE);
                canMove = false;
                hasCollision = true;
                break;
            }
        }
    }

    if (!hasCollision && position_.y + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
    {
        SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
        canMove = false;
    }

    if (!canMove) 
    {
        if (isInitialized_) 
        {
            blocks[indexY_][indexX_] = nullptr;
        }

        indexY_ = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(position_.y / Constants::Block::SIZE);
        blocks[indexY_][indexX_] = this;
        isInitialized_ = true;

        SetState(BlockState::Stationary);
    }
}

void IceBlock::Render() 
{
    if (!is_visible_ || !texture_)
    {
        return;
    }

    texture_->SetAlpha(static_cast<uint8_t>(alpha_));

    if (isScaled_) 
    {
        texture_->RenderScaled(&sourceRect_, &destination_rect_, rotationAngle_);
    }
    else 
    {
        texture_->Render(position_.x, position_.y, &sourceRect_);
    }
}