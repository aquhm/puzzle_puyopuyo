#include "IceBlock.hpp"
#include "../../core/GameApp.hpp"
#include "../../states/GameState.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../utils/RectUtil.hpp"
#include "../../utils/Logger.hpp"


void IceBlock::SetState(BlockState state) 
{
    state_ = state;

    switch (state_) 
    {
    case BlockState::Stationary:
        source_rect_.x = block_origin_Position_.x;
        source_rect_.y = block_origin_Position_.y;
        down_velocity_ = 0.0f;
        accumulate_time_ = 0.0f;
        break;

    case BlockState::Destroying:
        alpha_ = 255.0f;
        accumulate_time_ = 0.0f;
        break;

    case BlockState::DownMoving:
        source_rect_.x = 193.0f;
        source_rect_.y = 385.0f;
        break;

    case BlockState::PlayOut:
        accumulate_time_ = 0.0f;
        rotation_angle_ = 0.0f;
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

    accumulate_time_ += deltaTime;

    if (accumulate_time_ <= DESTROY_DURATION) 
    {
        if (texture_ && alpha_ > 0) 
        {
            alpha_ = 255.0f - (accumulate_time_ * ALPHA_RATE);
        }
    }
}

void IceBlock::UpdateDownMoving(float deltaTime) 
{
    float fallSpeed = deltaTime * static_cast<float>(Constants::Board::BOARD_Y_COUNT - index_y_);

    down_velocity_ += fallSpeed * 0.1f;
    position_.y += down_velocity_;
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
        Block* block = blocks[y][index_x_];
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
        if (is_initialized_) 
        {
            if (index_y_ >= 0 && index_y_ < Constants::Board::BOARD_Y_COUNT &&
                index_x_ >= 0 && index_x_ < Constants::Board::BOARD_X_COUNT)
            {
                blocks[index_y_][index_x_] = nullptr;
            }
        }

        int newIndexY = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(position_.y / Constants::Block::SIZE);
        newIndexY = std::max<int>(0, std::min<int>(newIndexY, Constants::Board::BOARD_Y_COUNT - 1));

        if (newIndexY >= 0 && newIndexY < Constants::Board::BOARD_Y_COUNT &&
            index_x_ >= 0 && index_x_ < Constants::Board::BOARD_X_COUNT)
        {
            blocks[newIndexY][index_x_] = this;
            index_y_ = newIndexY;
            is_initialized_ = true;
            SetState(BlockState::Stationary);
        }       
    }
}

void IceBlock::Render() 
{
    if (!is_visible_ || !texture_)
    {
        return;
    }

    texture_->SetAlpha(static_cast<uint8_t>(alpha_));

    if (is_scaled_) 
    {
        texture_->RenderScaled(&source_rect_, &destination_rect_, rotation_angle_);
    }
    else 
    {
        texture_->Render(position_.x, position_.y, &source_rect_);
    }
}