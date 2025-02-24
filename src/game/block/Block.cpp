#include "Block.hpp"

#include "../../core/common/constants/Constants.hpp"
#include "../../core/common/types/GameTypes.hpp"
#include "../../core/GameApp.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../states/GameState.hpp"
#include "../system/GamePlayer.hpp"

#include <stdexcept>

Block::Block() 
{
    sourceRect_ = { 1, 1, static_cast<int>(Constants::Block::SIZE), static_cast<int>(Constants::Block::SIZE) };
    blockOriginPos_ = { 1, 1 };

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
        sourceRect_ = other.sourceRect_;
        blockOriginPos_ = other.blockOriginPos_;
        std::copy(std::begin(other.blockEffectPos_), std::end(other.blockEffectPos_), std::begin(blockEffectPos_));

        blockType_ = other.blockType_;
        state_ = other.state_;
        linkState_ = other.linkState_;
        effectState_ = other.effectState_;

        level_ = other.level_;
        texture_ = other.texture_;

        isScaled_ = other.isScaled_;
        isRecursionCheck_ = other.isRecursionCheck_;
        isStandard_ = other.isStandard_;
        isChanged_ = other.isChanged_;

        indexX_ = other.indexX_;
        indexY_ = other.indexY_;

        accumTime_ = other.accumTime_;
        accumEffectTime_ = other.accumEffectTime_;
        rotationAngle_ = other.rotationAngle_;
        scaleVelocity_ = other.scaleVelocity_;
        downVelocity_ = other.downVelocity_;

        playerID_ = other.playerID_;
    }
    return *this;
}

void Block::InitializeEffectPositions() 
{
    // 각 이펙트 상태별 위치 초기화
    blockEffectPos_[static_cast<int>(EffectState::Sparkle)] = { 1, 1 + (static_cast<int>(Constants::Block::SIZE) + 1) * 9 };
    blockEffectPos_[static_cast<int>(EffectState::Compress)] = { 1, 1 };
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
    if (!isStandard_) return;

    accumTime_ += deltaTime;

    if (accumTime_ >= Constants::Block::CHANGE_TIME)
    {
        accumTime_ = 0.0f;
        isChanged_ = !isChanged_;

        sourceRect_.x = isChanged_ ?
            blockEffectPos_[static_cast<int>(EffectState::Sparkle)].x :
            blockOriginPos_.x;

        sourceRect_.y = isChanged_ ?
            blockEffectPos_[static_cast<int>(EffectState::Sparkle)].y :
            blockOriginPos_.y;
    }
}

void Block::UpdateBlockEffect(float deltaTime) 
{
    accumEffectTime_ += deltaTime;

    if (effectState_ == EffectState::Compress) 
    {
        if (accumEffectTime_ <= Constants::Block::EFFECT_COMPRESS_TIME)
        {
            sourceRect_.x = blockEffectPos_[static_cast<int>(EffectState::Compress)].x;
            sourceRect_.y = blockEffectPos_[static_cast<int>(EffectState::Compress)].y;
        }
        else 
        {
            SetEffectState(EffectState::Expand);
            accumEffectTime_ = 0.0f;
        }
    }
    else if (effectState_ == EffectState::Expand) 
    {
        if (accumEffectTime_ <= Constants::Block::EFFECT_EXPAND_TIME)
        {
            sourceRect_.x = blockEffectPos_[static_cast<int>(EffectState::Compress)].x +
                static_cast<int>(Constants::Block::SIZE);
            sourceRect_.y = blockEffectPos_[static_cast<int>(EffectState::Compress)].y;
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
    if (effectState_ != EffectState::Destroy || accumEffectTime_ > 1.0f) 
    {
        SetState(BlockState::PlayOut);
        return;
    }

    accumEffectTime_ += deltaTime;

    int destroyIndex = static_cast<int>(EffectState::Destroy);

    sourceRect_.x = blockEffectPos_[destroyIndex].x;
    sourceRect_.y = blockEffectPos_[destroyIndex].y;

    if (accumEffectTime_ <= Constants::Block::DESTROY_EXPAND_TIME)
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

    UpdateDestinationRect();
}

void Block::UpdateDestroyingRotate(float deltaTime) 
{
    rotationAngle_ = (360.0f * (accumEffectTime_ - Constants::Block::DESTROY_EXPAND_TIME)) / Constants::Block::EFFECT_DESTROY_TIME;

    float scaleDelta = Constants::Block::DESTROY_DELTA_SIZE / 360.0f * rotationAngle_;
    float posDelta = Constants::Block::DESTROY_POS_VELOCITY * deltaTime;

    SetSize(Constants::Block::SIZE - scaleDelta, Constants::Block::SIZE - scaleDelta);
    position_.x += posDelta;
    position_.y += posDelta;

    UpdateDestinationRect();

    if (rotationAngle_ >= 360.0f) 
    {
        SetState(BlockState::PlayOut);
    }
}

void Block::UpdateDownMoving(float deltaTime)
{
    // 낙하 속도 계산 및 위치 업데이트
    float fallSpeed = deltaTime * (static_cast<float>(Constants::Board::BOARD_Y_COUNT) + Constants::Block::SHATTERING_DOWN_SPEED -static_cast<float>(indexY_));

    downVelocity_ += fallSpeed;
    position_.y += downVelocity_;

    SetY(position_.y);

    // 게임 보드 블록 배열 가져오기
    Block* (*blocks)[Constants::Board::BOARD_X_COUNT] = nullptr;

    if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
    {        
        blocks = (playerID_ != 0) ? gameState->GetGameBlocks(): gameState->GetPlayer()->GetGameBlocks();
    }

    if (!blocks)
    {
        return;
    }


    // 충돌 검사
    bool hasCollision = false;
    bool canMove = true;

    for (int y = 0; y < Constants::Board::HEIGHT; ++y) 
    {
        Block* targetBlock = blocks[y][indexX_];

        if (!targetBlock || targetBlock == this)
        {
            continue;
        }

        if (targetBlock->GetState() != BlockState::Stationary) 
        {
            continue;
        }

        SDL_FRect intersectResult;

        if (SDL_GetRectIntersectionFloat(&destination_rect_, &targetBlock->GetRect(), &intersectResult) == true) 
        {
            SetY(intersectResult.y - Constants::Block::SIZE);
            canMove = false;
            hasCollision = true;
            break;
        }
    }

    // 바닥 충돌 검사
    if (!hasCollision && position_.y + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
    {
        SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
        canMove = false;
    }

    // 이동 불가능한 경우 처리
    if (canMove == false) 
    {
        // 기존 위치의 블록 제거
        blocks[indexY_][indexX_] = nullptr;

        // 새 Y 인덱스 계산 및 위치 업데이트
        indexY_ = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(position_.y / Constants::Block::SIZE);

        // 현재 블록을 새 위치에 설정
        blocks[indexY_][indexX_] = this;

        // 상태 변경
        SetState(BlockState::Stationary);
    }    
}

void Block::Render() 
{
    if (!is_visible_ || !texture_)
    {
        return;
    }

    if (isScaled_) 
    {
        texture_->RenderScaled(&sourceRect_, &destination_rect_, rotationAngle_);
    }
    else 
    {
        texture_->Render(position_.x, position_.y, &sourceRect_);
    }
}

void Block::Release() 
{
    texture_.reset();
}

void Block::SetBlockType(BlockType type) 
{
    blockType_ = type;
    
    int blockSize = static_cast<int>(Constants::Block::SIZE);

    const float baseX = static_cast<float>(1);
    const float blockOffset = static_cast<float>(blockSize + 1);

    // 블록 타입에 따른 텍스처 위치 설정
    switch (blockType_) 
    {
    case BlockType::Red:
        sourceRect_.y = baseX;
        blockOriginPos_.y = sourceRect_.y;
        blockEffectPos_[static_cast<int>(EffectState::Sparkle)].x = baseX;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 11;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 9;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].x = baseX;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 12;
        break;

    case BlockType::Green:
        sourceRect_.y = baseX + (Constants::Block::SIZE + baseX);
        blockOriginPos_.y = sourceRect_.y;
        blockEffectPos_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 13;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 9;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].x = baseX;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 13;
        break;

    case BlockType::Blue:
        sourceRect_.y = baseX + (Constants::Block::SIZE + baseX) * 2;
        blockOriginPos_.y = sourceRect_.y;
        blockEffectPos_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset * 2;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].x = baseX;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 10;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].x = baseX + blockOffset * 2;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 12;
        break;

    case BlockType::Yellow:
        sourceRect_.y = baseX + (Constants::Block::SIZE + baseX) * 3;
        blockOriginPos_.y = sourceRect_.y;
        blockEffectPos_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset * 3;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 2;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 10;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].x = baseX + blockOffset * 2;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 13;
        break;

    case BlockType::Purple:
        sourceRect_.y = baseX + (Constants::Block::SIZE + baseX) * 4;
        blockOriginPos_.y = sourceRect_.y;
        blockEffectPos_[static_cast<int>(EffectState::Sparkle)].x = baseX + blockOffset * 4;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].x = baseX + blockOffset * 4;
        blockEffectPos_[static_cast<int>(EffectState::Compress)].y = baseX + blockOffset * 10;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].x = baseX + blockOffset * 4;
        blockEffectPos_[static_cast<int>(EffectState::Destroy)].y = baseX + blockOffset * 12;
        break;

    case BlockType::Ice:
        sourceRect_.x = baseX + blockOffset * 6;
        sourceRect_.y = baseX + blockOffset * 12;
        blockOriginPos_.y = sourceRect_.y;
        blockOriginPos_.x = sourceRect_.x;
        break;
    }
}

void Block::SetSize(float width, float height)
{
    RenderableObject::SetSize(width, height);
    UpdateDestinationRect();
    isScaled_ = true;
}


void Block::SetState(BlockState state) 
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

    case BlockState::Effecting:
        SetEffectState(EffectState::Compress);
        accumEffectTime_ = 0.0f;
        break;

    case BlockState::Destroying:
        SetEffectState(EffectState::Destroy);
        accumEffectTime_ = 0.0f;
        break;

    case BlockState::DownMoving:
        UpdateLinkStateForDownMoving();
        break;

    case BlockState::PlayOut:
        SetEffectState(EffectState::None);
        accumEffectTime_ = 0.0f;
        rotationAngle_ = 0.0f;
        break;
    }
}

void Block::UpdateLinkStateForDownMoving() 
{
    // 현재 블록이 좌우 연결 상태를 가지고 있는지 확인
    const bool hasHorizontalLinks =(static_cast<int>(linkState_) &
            (static_cast<int>(LinkState::Left) | static_cast<int>(LinkState::Right))) != 0;

    if (!hasHorizontalLinks) 
    {
        // 좌우 연결이 없으면 처리할 필요 없음
        SetLinkState(LinkState::Normal);
        return;
    }

    Block* (*gameBoard)[Constants::Board::BOARD_X_COUNT] = nullptr;
    // 게임 보드에서 연결된 블록들의 링크 상태 업데이트
    if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
    {
        gameBoard = (playerID_ != 0) ? gameState->GetGameBlocks() : gameState->GetPlayer()->GetGameBlocks();
    }
    
    if (!gameBoard)
    {
        return;
    }

    // 좌측 블록 체크
    if (indexX_ > 0 && (static_cast<int>(linkState_) & static_cast<int>(LinkState::Left))) 
    {
        if (auto leftBlock = gameBoard[indexY_][indexX_ - 1])
        {
            if (leftBlock->GetBlockType() == blockType_) 
            {
                // 좌측 블록의 우측 링크 제거
                auto leftLinkState = leftBlock->GetLinkState();
                leftLinkState = static_cast<LinkState>(static_cast<int>(leftLinkState) & ~static_cast<int>(LinkState::Right));
                leftBlock->SetLinkState(leftLinkState);
            }
        }
    }

    // 우측 블록 체크
    if (indexX_ < Constants::Board::BOARD_X_COUNT - 1 && (static_cast<int>(linkState_) & static_cast<int>(LinkState::Right))) 
    {
        if (auto rightBlock = gameBoard[indexY_][indexX_ + 1])
        {
            if (rightBlock->GetBlockType() == blockType_) 
            {
                // 우측 블록의 좌측 링크 제거
                auto rightLinkState = rightBlock->GetLinkState();
                rightLinkState = static_cast<LinkState>(static_cast<int>(rightLinkState) & ~static_cast<int>(LinkState::Left));
                rightBlock->SetLinkState(rightLinkState);
            }
        }
    }

    // 현재 블록의 링크 상태 초기화
    SetLinkState(LinkState::Normal);
}

void Block::SetLinkState(LinkState state) 
{
    linkState_ = state;
    UpdateSourceRectForLinkState();
}

void Block::UpdateSourceRectForLinkState() 
{
    // 링크 상태에 따른 sourceRect_.x 값 설정
    const int baseOffset = static_cast<int>(Constants::Block::SIZE) + 1;

    switch (linkState_) 
    {
    case LinkState::Normal:          sourceRect_.x = 1; break;
    case LinkState::Left:           sourceRect_.x = 1 + baseOffset * 4; break;
    case LinkState::Top:            sourceRect_.x = 1 + baseOffset; break;
    case LinkState::Right:          sourceRect_.x = 1 + baseOffset * 8; break;
    case LinkState::Bottom:         sourceRect_.x = 1 + baseOffset * 2; break;
    case LinkState::RightTop:       sourceRect_.x = 1 + baseOffset * 9; break;
    case LinkState::RightBottom:    sourceRect_.x = 1 + baseOffset * 10; break;
    case LinkState::TopBottom:      sourceRect_.x = 1 + baseOffset * 3; break;
    case LinkState::LeftTop:        sourceRect_.x = 1 + baseOffset * 5; break;
    case LinkState::LeftBottom:     sourceRect_.x = 1 + baseOffset * 6; break;
    case LinkState::LeftRight:      sourceRect_.x = 1 + baseOffset * 12; break;
    case LinkState::RightTopBottom: sourceRect_.x = 1 + baseOffset * 11; break;
    case LinkState::LeftTopBottom:  sourceRect_.x = 1 + baseOffset * 7; break;
    case LinkState::LeftRightTop:   sourceRect_.x = 1 + baseOffset * 13; break;
    case LinkState::LeftRightBottom:sourceRect_.x = 1 + baseOffset * 14; break;
    case LinkState::LeftRightTopBottom: sourceRect_.x = 1 + baseOffset * 15; break;
    default: break;
    }
}

bool Block::operator<(const Block& rhs) const
{
    if (indexY_ < rhs.indexY_)
    {
        return true;
    }

    if (indexY_ == rhs.indexY_)
    {
        return indexX_ < rhs.indexX_;
    }

    return false;
}