#include "Block.hpp"

#include "../../core/common/constants/Constants.hpp"
#include "../../core/common/types/GameTypes.hpp"
#include "../../core/GameApp.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../states/GameState.hpp"
#include "../system/LocalPlayer.hpp"
#include "../system/RemotePlayer.hpp"


#include <stdexcept>
#include "../../utils/RectUtil.hpp"

Block::Block() 
{
    source_rect_ = { 1, 1,Constants::Block::SIZE, Constants::Block::SIZE };
    block_origin_Position_ = { 1, 1 };

    InitializeEffectPositions();
}

Block::Block(const Block& other)
{
    *this = other;
}

Block& Block::operator=(const Block& other) 
{
    if (this != &other) 
    {
        auto temp = other.Clone();
    
        std::swap(*this, *temp);
    }

    return *this;
}

std::shared_ptr<Block> Block::Clone() const
{
    auto newBlock = std::make_shared<Block>();

    newBlock->position_ = position_;
    newBlock->size_ = size_;
    newBlock->destination_rect_ = destination_rect_;
    newBlock->is_visible_ = is_visible_;

    newBlock->source_rect_ = source_rect_;
    newBlock->block_origin_Position_ = block_origin_Position_;

    for (size_t i = 0; i < static_cast<size_t>(EffectState::Max); i++) 
    {
        newBlock->block_effect_position_[i] = block_effect_position_[i];
    }

    newBlock->block_type_ = block_type_;
    newBlock->state_ = state_;
    newBlock->link_state_ = link_state_;
    newBlock->effect_state_ = effect_state_;

    newBlock->level_ = level_;
    newBlock->texture_ = texture_;

    newBlock->is_scaled_ = is_scaled_;
    newBlock->is_recursionCheck_ = is_recursionCheck_;
    newBlock->is_standard_ = is_standard_;
    newBlock->is_changed_ = is_changed_;

    newBlock->index_x_ = index_x_;
    newBlock->index_y_ = index_y_;

    newBlock->accumulate_time_ = accumulate_time_;
    newBlock->accumulate_effect_time_ = accumulate_effect_time_;
    newBlock->rotation_angle_ = rotation_angle_;
    newBlock->scale_velocity_ = scale_velocity_;
    newBlock->down_velocity_ = down_velocity_;

    newBlock->playerID_ = playerID_;

    return newBlock;
}

void Block::InitializeEffectPositions() 
{
    block_effect_position_[static_cast<int>(EffectState::Sparkle)] = { 1, 1 + (static_cast<int>(Constants::Block::SIZE) + 1) * 9 };
    block_effect_position_[static_cast<int>(EffectState::Compress)] = { 1, 1 };
}

void Block::Update(float deltaTime) 
{
    switch (state_) 
    {
    case BlockState::Playing:
        UpdatePlayingState(deltaTime);
        break;
    case BlockState::Effecting:
        UpdateBlockEffect(deltaTime);
        break;
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

void Block::UpdatePlayingState(float deltaTime) 
{
    if (!is_standard_)
    {
        return;
    }

    accumulate_time_ += deltaTime;

    if (accumulate_time_ >= Constants::Block::CHANGE_TIME)
    {
        accumulate_time_ = 0.0f;
        is_changed_ = !is_changed_;

        source_rect_.x = is_changed_ ? block_effect_position_[static_cast<int>(EffectState::Sparkle)].x : block_origin_Position_.x;

        source_rect_.y = is_changed_ ? block_effect_position_[static_cast<int>(EffectState::Sparkle)].y :block_origin_Position_.y;
    }
}

void Block::UpdateBlockEffect(float deltaTime) 
{
    accumulate_effect_time_ += deltaTime;

    if (effect_state_ == EffectState::Compress) 
    {
        if (accumulate_effect_time_ <= Constants::Block::EFFECT_COMPRESS_TIME)
        {
            source_rect_.x = block_effect_position_[static_cast<int>(EffectState::Compress)].x;
            source_rect_.y = block_effect_position_[static_cast<int>(EffectState::Compress)].y;
        }
        else 
        {
            SetEffectState(EffectState::Expand);
            accumulate_effect_time_ = 0.0f;
        }
    }
    else if (effect_state_ == EffectState::Expand) 
    {
        if (accumulate_effect_time_ <= Constants::Block::EFFECT_EXPAND_TIME)
        {
            source_rect_.x = block_effect_position_[static_cast<int>(EffectState::Compress)].x + static_cast<int>(Constants::Block::SIZE);
            source_rect_.y = block_effect_position_[static_cast<int>(EffectState::Compress)].y;        
        }
        else 
        {
            SetEffectState(EffectState::None);
            SetState(BlockState::Stationary);
        }
    }
}

void Block::UpdateDestroying(float deltaTime) 
{
    if (effect_state_ != EffectState::Destroy || accumulate_effect_time_ > 1.0f) 
    {
        SetState(BlockState::PlayOut);
        return;
    }

    accumulate_effect_time_ += deltaTime;

    int destroyIndex = static_cast<int>(EffectState::Destroy);

    source_rect_.x = block_effect_position_[destroyIndex].x;
    source_rect_.y = block_effect_position_[destroyIndex].y;

    if (accumulate_effect_time_ <= Constants::Block::DESTROY_EXPAND_TIME)
    {
        UpdateDestroyingExpand(deltaTime);
    }
    else 
    {
        UpdateDestroyingRotate(deltaTime);
    }
}

void Block::UpdateDestroyingExpand(float deltaTime)
{
    float scaleVelocity = Constants::Block::DESTROY_EXPAND_DELTA_SIZE / Constants::Block::DESTROY_EXPAND_TIME * deltaTime;
    float posVelocity = Constants::Block::DESTROY_EXPAND_POS_VELOCITY * deltaTime;

    size_.x += scaleVelocity;
    size_.y += scaleVelocity;
    position_.x -= posVelocity;
    position_.y -= posVelocity;

    SetScale(size_.x, size_.y);
    SetPosition(position_.x, position_.y);
    //UpdateDestinationRect();
}

void Block::UpdateDestroyingRotate(float deltaTime) 
{
    rotation_angle_ = (360.0f * (accumulate_effect_time_ - Constants::Block::DESTROY_EXPAND_TIME)) / Constants::Block::EFFECT_DESTROY_TIME;

    float scaleDelta = Constants::Block::DESTROY_DELTA_SIZE / 360.0f * rotation_angle_;
    float posDelta = Constants::Block::DESTROY_POS_VELOCITY * deltaTime;

    SetScale(Constants::Block::SIZE - scaleDelta, Constants::Block::SIZE - scaleDelta);

    position_.x += posDelta;
    position_.y += posDelta;

    //UpdateDestinationRect();
    SetPosition(position_.x, position_.y);

    if (rotation_angle_ >= 360.0f) 
    {
        SetState(BlockState::PlayOut);
    }
}

void Block::UpdateDownMoving(float deltaTime)
{
    float fallSpeed = deltaTime * (static_cast<float>(Constants::Board::BOARD_Y_COUNT) + Constants::Block::SHATTERING_DOWN_SPEED - static_cast<float>(index_y_));

    down_velocity_ += fallSpeed;
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

    SDL_Rect intersectResult;
    SDL_Rect destinationRect;
    SDL_Rect targetRect;

    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; ++y) 
    {
        Block* targetBlock = blocks[y][index_x_];

        if (!targetBlock || targetBlock == this)
        {
            continue;
        }

        if (targetBlock->GetState() != BlockState::Stationary) 
        {
            continue;
        }
       
        RectUtils::ConvertFRectToRect(destination_rect_, &destinationRect);
        RectUtils::ConvertFRectToRect(targetBlock->GetRect(), &targetRect);

        if (SDL_GetRectIntersection(&destinationRect, &targetRect, &intersectResult) == true)
        {
            SetY(intersectResult.y - Constants::Block::SIZE);
            canMove = false;
            hasCollision = true;
            break;
        }
    }

    if (!hasCollision && position_.y + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
    {
        SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
        canMove = false;
    }

    if (canMove == false) 
    {
        blocks[index_y_][index_x_] = nullptr;

        index_y_ = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(position_.y / Constants::Block::SIZE);

        blocks[index_y_][index_x_] = this;

        SetState(BlockState::Stationary);
    }    
}

void Block::Render() 
{
    if (!is_visible_ || !texture_)
    {
        return;
    }

    texture_->SetAlpha(255);

    if (is_scaled_) 
    {
        texture_->RenderScaled(&source_rect_, &destination_rect_, rotation_angle_);
    }
    else 
    {
        texture_->Render(position_.x, position_.y, &source_rect_);
    }
}

void Block::Release() 
{
    texture_.reset();
}

void Block::SetBlockType(BlockType type) 
{
    block_type_ = type;
    
    int blockSize = static_cast<int>(Constants::Block::SIZE);

    const float baseX = static_cast<float>(1);
    const float blockOffset = static_cast<float>(blockSize + 1);

    switch (block_type_) 
    {
    case BlockType::Red:
        source_rect_.y = baseX;
        block_origin_Position_.y = source_rect_.y;
        block_effect_position_[static_cast<int>(EffectState::Sparkle)].x = baseX;
        block_effect_position_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 11;
        block_effect_position_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 9;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].x = baseX;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 12;
        break;

    case BlockType::Green:
        source_rect_.y = baseX + (Constants::Block::SIZE + baseX);
        block_origin_Position_.y = source_rect_.y;
        block_effect_position_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset;
        block_effect_position_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 13;
        block_effect_position_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 9;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].x = baseX;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 13;
        break;

    case BlockType::Blue:
        source_rect_.y = baseX + (Constants::Block::SIZE + baseX) * 2;
        block_origin_Position_.y = source_rect_.y;
        block_effect_position_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset * 2;
        block_effect_position_[static_cast<int>(EffectState::Compress)].x = baseX;
        block_effect_position_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 10;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].x = baseX + blockOffset * 2;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 12;
        break;

    case BlockType::Yellow:
        source_rect_.y = baseX + (Constants::Block::SIZE + baseX) * 3;
        block_origin_Position_.y = source_rect_.y;
        block_effect_position_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset * 3;
        block_effect_position_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 2;
        block_effect_position_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 10;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].x = baseX + blockOffset * 2;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 13;
        break;

    case BlockType::Purple:
        source_rect_.y = baseX + (Constants::Block::SIZE + baseX) * 4;
        block_origin_Position_.y = source_rect_.y;
        block_effect_position_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset * 4;
        block_effect_position_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 4;
        block_effect_position_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 10;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].x = baseX + blockOffset * 4;
        block_effect_position_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 12;
        break;

    case BlockType::Ice:
        source_rect_.x = 193;
        source_rect_.y = 385;
        block_origin_Position_.y = source_rect_.y;
        block_origin_Position_.x = source_rect_.x;
        break;
    }
}

void Block::SetScale(float width, float height)
{
    RenderableObject::SetScale(width, height);
    UpdateDestinationRect();
    is_scaled_ = true;
}

void Block::SetState(BlockState state) 
{
    state_ = state;

    switch (state_) 
    {
    case BlockState::Stationary:
        source_rect_.x = block_origin_Position_.x;
        source_rect_.y = block_origin_Position_.y;
        down_velocity_ = 0.0f;
        accumulate_effect_time_ = 0.0f;
        break;

    case BlockState::Effecting:
        SetEffectState(EffectState::Compress);
        accumulate_effect_time_ = 0.0f;
        break;

    case BlockState::Destroying:
        SetEffectState(EffectState::Destroy);
        accumulate_effect_time_ = 0.0f;
        break;

    case BlockState::DownMoving:
        UpdateLinkStateForDownMoving();
        break;

    case BlockState::PlayOut:
        SetEffectState(EffectState::None);
        accumulate_effect_time_ = 0.0f;
        rotation_angle_ = 0.0f;
        break;
    }
}

void Block::UpdateLinkStateForDownMoving()
{
    // 현재 블록이 좌우 연결 상태를 가지고 있는지 확인
    const bool hasHorizontalLinks = (static_cast<int>(link_state_) & (static_cast<int>(LinkState::Left) | static_cast<int>(LinkState::Right))) != 0;

    // 수평 링크가 있는 경우에만 이웃 블록 상태 업데이트 수행
    if (hasHorizontalLinks)
    {
        Block* (*gameBoard)[Constants::Board::BOARD_X_COUNT] = nullptr;
        if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            gameBoard = gameState->GetGameBlocks(playerID_);
        }

        if (gameBoard)
        {
            // 좌측 블록 체크
            if (index_x_ > 0)
            {
                if (auto leftBlock = gameBoard[index_y_][index_x_ - 1])
                {
                    if (leftBlock->GetBlockType() == block_type_)
                    {
                        // 이전 코드와 같이 XOR 연산 사용
                        auto leftLinkState = leftBlock->GetLinkState();
                        leftLinkState = static_cast<LinkState>(static_cast<int>(leftLinkState) ^ static_cast<int>(LinkState::Right));
                        leftBlock->SetLinkState(leftLinkState);
                    }
                }
            }

            // 우측 블록 체크
            if (index_x_ < Constants::Board::BOARD_X_COUNT - 1)
            {
                if (auto rightBlock = gameBoard[index_y_][index_x_ + 1])
                {
                    if (rightBlock->GetBlockType() == block_type_)
                    {
                        // 이전 코드와 같이 XOR 연산 사용
                        auto rightLinkState = rightBlock->GetLinkState();
                        rightLinkState = static_cast<LinkState>(static_cast<int>(rightLinkState) ^ static_cast<int>(LinkState::Left));
                        rightBlock->SetLinkState(rightLinkState);
                    }
                }
            }
        }
    }

    // 현재 블록의 링크 상태 초기화
    SetLinkState(LinkState::Normal);
}

void Block::SetLinkState(LinkState state) 
{
    link_state_ = state;
    UpdateSourceRectForLinkState();
}

void Block::UpdateSourceRectForLinkState() 
{
    // 링크 상태에 따른 source_rect_.x 값 설정
    const int baseOffset = static_cast<int>(Constants::Block::SIZE) + 1;

    switch (link_state_) 
    {
    case LinkState::Normal:         source_rect_.x = 1; break;
    case LinkState::Left:           source_rect_.x = 1 + baseOffset * 4; break;
    case LinkState::Top:            source_rect_.x = 1 + baseOffset; break;
    case LinkState::Right:          source_rect_.x = 1 + baseOffset * 8; break;
    case LinkState::Bottom:         source_rect_.x = 1 + baseOffset * 2; break;
    case LinkState::RightTop:       source_rect_.x = 1 + baseOffset * 9; break;
    case LinkState::RightBottom:    source_rect_.x = 1 + baseOffset * 10; break;
    case LinkState::TopBottom:      source_rect_.x = 1 + baseOffset * 3; break;
    case LinkState::LeftTop:        source_rect_.x = 1 + baseOffset * 5; break;
    case LinkState::LeftBottom:     source_rect_.x = 1 + baseOffset * 6; break;
    case LinkState::LeftRight:      source_rect_.x = 1 + baseOffset * 12; break;
    case LinkState::RightTopBottom: source_rect_.x = 1 + baseOffset * 11; break;
    case LinkState::LeftTopBottom:  source_rect_.x = 1 + baseOffset * 7; break;
    case LinkState::LeftRightTop:   source_rect_.x = 1 + baseOffset * 13; break;
    case LinkState::LeftRightBottom:source_rect_.x = 1 + baseOffset * 14; break;
    case LinkState::LeftRightTopBottom: source_rect_.x = 1 + baseOffset * 15; break;
    default: break;
    }
}

bool Block::operator<(const Block& rhs) const
{
    if (index_y_ < rhs.index_y_)
    {
        return true;
    }

    if (index_y_ == rhs.index_y_)
    {
        return index_x_ < rhs.index_x_;
    }

    return false;
}