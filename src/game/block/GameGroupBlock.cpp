#include "GameGroupBlock.hpp"

#include "../../states/GameState.hpp"
#include "../../network/NetworkController.hpp"
#include "../../network/player/Player.hpp"

#include "../../core/GameApp.hpp"
#include "../../core/GameUtils.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/common/types/GameTypes.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../core/manager/PlayerManager.hpp"
#include "../system/LocalPlayer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "../../utils/Logger.hpp"
#include "../../utils/RectUtil.hpp"


GameGroupBlock::GameGroupBlock()
    : GroupBlock()
    , rotateState_(RotateState::Default)
    , isFalling_(false)
    , isRotating_(false)
    , checkingCollision_(false)
    , canMove_(true)
    , fallingIdx_(-1)
    , blockIndexX_(0)
    , playerID_(0)
    , updateTime_(0)
    , velocity_(0.0f)
    , addVelocity_(1.0f)
    , rotateVelocity_(0.0f)
    , horizontalVelocity_(0.0f)
    
{
}

void GameGroupBlock::SetPosX(float x) 
{
    position_.x = x;
    blockIndexX_ = static_cast<int>((position_.x - Constants::Board::WIDTH_MARGIN) / Constants::Block::SIZE);

    if (blocks_[0]) 
    {
        blocks_[0]->SetX(position_.x);
        blocks_[0]->SetPosIdx_X(blockIndexX_);
    }

    if (blocks_[1]) 
    {
        switch (rotateState_) 
        {
        case RotateState::Default:
        case RotateState::Top:
            blocks_[1]->SetX(position_.x);
            blocks_[1]->SetPosIdx_X(blockIndexX_);
            break;

        case RotateState::Right:
            blocks_[1]->SetX(position_.x + Constants::Block::SIZE);
            blocks_[1]->SetPosIdx_X(blockIndexX_ + 1);
            break;

        case RotateState::Left:
            blocks_[1]->SetX(position_.x - Constants::Block::SIZE);
            blocks_[1]->SetPosIdx_X(blockIndexX_ - 1);
            break;
        }
    }

    UpdateDestRect();
}

void GameGroupBlock::SetPosY(float y) 
{
    position_.y = y;

    if (blocks_[0]) {
        blocks_[0]->SetY(position_.y);
    }

    if (blocks_[1]) {
        switch (rotateState_) {
        case RotateState::Default:
            blocks_[1]->SetY(position_.y + Constants::Block::SIZE);
            break;

        case RotateState::Top:
            blocks_[1]->SetY(position_.y - Constants::Block::SIZE);
            break;

        case RotateState::Right:
        case RotateState::Left:
            blocks_[1]->SetY(position_.y);
            break;
        }
    }

    UpdateDestRect();
}

void GameGroupBlock::SetPosXY(float x, float y) 
{
    position_.x = x;
    position_.y = y;

    if (!blocks_[0] || !blocks_[1]) return;

    blocks_[0]->SetPosition(position_.x, position_.y);
    blockIndexX_ = static_cast<int>((position_.x - Constants::Board::WIDTH_MARGIN) / Constants::Block::SIZE);
    blocks_[0]->SetPosIdx_X(blockIndexX_);

    switch (rotateState_) {
    case RotateState::Default:
        blocks_[1]->SetPosition(position_.x, position_.y + Constants::Block::SIZE);
        blocks_[1]->SetPosIdx_X(blockIndexX_);
        break;

    case RotateState::Top:
        blocks_[1]->SetPosition(position_.x, position_.y - Constants::Block::SIZE);
        blocks_[1]->SetPosIdx_X(blockIndexX_);
        break;

    case RotateState::Right:
        blocks_[1]->SetPosition(position_.x + Constants::Block::SIZE, position_.y);
        blocks_[1]->SetPosIdx_X(blockIndexX_ + 1);
        break;

    case RotateState::Left:
        blocks_[1]->SetPosition(position_.x - Constants::Block::SIZE, position_.y);
        blocks_[1]->SetPosIdx_X(blockIndexX_ - 1);
        break;
    }

    UpdateDestRect();
}

void GameGroupBlock::MoveLeft(bool collisionCheck) 
{
    if (isRotating_ || isFalling_ || checkingCollision_ || !canMove_) 
    {
        return;
    }

    if (collisionCheck) 
    {
        
        SDL_Rect leftCollRects[2];
        SDL_Rect targetRect;

        GetCollisionRect(blocks_[Standard].get(), &leftCollRects[0], Constants::Direction::Left);
        GetCollisionRect(blocks_[Satellite].get(), &leftCollRects[1], Constants::Direction::Left);

        bool canMove = true;

        // 충돌 체크
        for (const auto& block : *gameBlockList_) 
        {
            RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

            if (SDL_HasRectIntersection(&leftCollRects[0], &targetRect) == true ||
                SDL_HasRectIntersection(&leftCollRects[1], &targetRect) == true)
            {
                canMove = false;
                break;
            }
        }

        // 경계 체크
        float limit = Constants::Board::WIDTH_MARGIN;
        if (rotateState_ == RotateState::Left) 
        {
            limit += Constants::Block::SIZE;
        }

        if (canMove && position_.x > limit)
        {
            if (auto stateManager = GAME_APP.GetStateManager().GetCurrentState()) 
            {
                // GameState 타입으로 캐스팅 시도
                if (auto gameState = dynamic_cast<GameState*>(stateManager.get())) 
                {
                    if (blockIndexX_ > 0 && gameState->GetLocalPlayer()->IsPossibleMove(blockIndexX_ - 1)) 
                    {
                        position_.x -= Constants::Block::SIZE;

                        SetPosX(position_.x);

                        gameState->GetLocalPlayer()->UpdateTargetPosIdx();

                        if (NETWORK.IsRunning())
                        {
                            NETWORK.MoveBlock(static_cast<uint8_t>(Constants::Direction::Left), position_.x);
                        }
                    }
                }
            }
        }
    }
    else 
    {
        position_.x -= Constants::Block::SIZE;
        SetPosX(position_.x);
    }
}

void GameGroupBlock::MoveRight(bool collisionCheck) 
{
    if (isRotating_ || isFalling_ || checkingCollision_ || !canMove_) 
    {
        return;
    }

    if (collisionCheck) 
    {   
        SDL_Rect rightCollRects[2];
        SDL_Rect targetRect;

        GetCollisionRect(blocks_[Standard].get(), &rightCollRects[0], Constants::Direction::Right);
        GetCollisionRect(blocks_[Satellite].get(), &rightCollRects[1], Constants::Direction::Right);

        bool canMove = true;

        for (const auto& block : *gameBlockList_) 
        {
            RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

            if (SDL_GetRectIntersection(&rightCollRects[Standard], &targetRect, &rightCollRects[0]) == true ||
                SDL_GetRectIntersection(&rightCollRects[Satellite], &targetRect, &rightCollRects[1]) == true)
            {
                canMove = false;
                break;
            }
        }

        float limit = Constants::Board::WIDTH - Constants::Board::WIDTH_MARGIN;
        if (rotateState_ == RotateState::Right) 
        {
            limit -= Constants::Block::SIZE;
        }

        if (canMove && position_.x + size_.x < limit) 
        {
            if (auto stateManager = GAME_APP.GetStateManager().GetCurrentState())
            {
                // GameState 타입으로 캐스팅 시도
                if (auto gameState = dynamic_cast<GameState*>(stateManager.get()))
                {
                    if (blockIndexX_ < Constants::Board::BOARD_X_COUNT - 1 && gameState->GetLocalPlayer()->IsPossibleMove(blockIndexX_ + 1)) 
                    {
                        position_.x += Constants::Block::SIZE;
                        SetPosX(position_.x);

                        gameState->GetLocalPlayer()->UpdateTargetPosIdx();

                        if (NETWORK.IsRunning())
                        {
                            NETWORK.MoveBlock(static_cast<uint8_t>(Constants::Direction::Right), position_.x);
                        }
                    }
                }
            }
        }
    }
    else 
    {
        position_.x += Constants::Block::SIZE;
        SetPosX(position_.x);
    }
}

bool GameGroupBlock::MoveDown(bool collisionCheck)
{
    checkingCollision_ = true;    

    bool hasCollision = false;
    bool canMove = true;
    SDL_Rect resultRect{};
    SDL_Rect controlRect{};
    SDL_Rect targetRect{};

    float standardY = blocks_[Standard]->GetY();
    float satelliteY = blocks_[Satellite]->GetY();

    switch (rotateState_) 
    {
    case RotateState::Default:
        // 위-아래 배치일 때의 충돌 체크
        for (const auto& block : *gameBlockList_) 
        {
            if (!block)
            {
                continue;
            }

            RectUtils::ConvertFRectToRect(blocks_[Satellite]->GetRect(), &controlRect);
            RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

            if (SDL_GetRectIntersection(&controlRect, &targetRect, &resultRect))
            {
                hasCollision = true;
                break;
            }
        }

        if (hasCollision) 
        {
            blocks_[Standard]->SetY(targetRect.y - Constants::Block::SIZE * 2);
            blocks_[Satellite]->SetY(targetRect.y - Constants::Block::SIZE);
            canMove_ = false;
        }
        else if (satelliteY + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
        {
            blocks_[Standard]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE * 2));
            blocks_[Satellite]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE));
            canMove_ = false;
        }
        break;

    case RotateState::Right:
    case RotateState::Left:
        if (isFalling_ == false) 
        {
            // 좌-우 배치일 때의 충돌 체크
            HandleHorizontalCollision();
        }
        else {
            // 블록 하나만 떨어지는 경우의 충돌 체크
            HandleSingleBlockFalling();
        }
        break;

    case RotateState::Top:
        // 아래-위 배치일 때의 충돌 체크
        for (const auto& block : *gameBlockList_) 
        {
            if (!block)
            {
                continue;
            }

            RectUtils::ConvertFRectToRect(blocks_[Standard]->GetRect(), &controlRect);
            RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

            if (SDL_GetRectIntersection(&controlRect, &targetRect, &resultRect))
            {
                hasCollision = true;
                break;
            }
        }

        if (hasCollision) 
        {
            blocks_[Standard]->SetY(targetRect.y - Constants::Block::SIZE);
            blocks_[Satellite]->SetY(targetRect.y - Constants::Block::SIZE * 2);
            canMove_ = false;
        }
        else if (satelliteY + Constants::Block::SIZE * 2 >= Constants::Board::HEIGHT) 
        {
            blocks_[Standard]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE));
            blocks_[Satellite]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE * 2));
            canMove_ = false;
        }
        break;
    }

    checkingCollision_ = false;
    return canMove_;
}

void GameGroupBlock::HandleHorizontalCollision() 
{
    bool collision1 = false;
    bool collision2 = false;
    SDL_Rect resultRect[2];
    SDL_Rect controlRect[2];
    SDL_Rect targetRect[2];

    for (const auto& block : *gameBlockList_) 
    {
        if (!block)
        {
            continue;
        }

        RectUtils::ConvertFRectToRect(blocks_[Standard]->GetRect(), &controlRect[0]);
        RectUtils::ConvertFRectToRect(blocks_[Satellite]->GetRect(), &controlRect[1]);
        RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect[0]);
        RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect[1]);

        if (SDL_GetRectIntersection(&controlRect[0], &targetRect[0], &resultRect[0]) == true)
        {
            SDL_RectToFRect(&resultRect[0], &intersectResultRect_[0]);
            collision1 = true;
        }                

        if (SDL_GetRectIntersection(&controlRect[1], &targetRect[1], &resultRect[1]) == true)
        {
            SDL_RectToFRect(&resultRect[1], &intersectResultRect_[1]);
            collision2 = true;
        }

        if (collision1 && collision2)
        {
            break;
        }
    }

    ProcessHorizontalCollisionResult(collision1, collision2);
}

void GameGroupBlock::ProcessHorizontalCollisionResult(bool collision1, bool collision2) 
{
    if (collision1 == true && collision2 == true)
    {
        blocks_[Standard]->SetY(intersectResultRect_[0].y - Constants::Block::SIZE);
        blocks_[Satellite]->SetY(intersectResultRect_[1].y - Constants::Block::SIZE);
        canMove_ = false;
    }
    else if (collision1 == true && collision2 == false) 
    {
        blocks_[Standard]->SetY(intersectResultRect_[0].y - Constants::Block::SIZE);
        fallingIdx_ = Satellite;
        isFalling_ = true;
        NETWORK.RequireFallingBlock(fallingIdx_, isFalling_);
    }
    else if (collision1 == false && collision2 == true) 
    {
        blocks_[Satellite]->SetY(intersectResultRect_[1].y - Constants::Block::SIZE);
        fallingIdx_ = Standard;
        isFalling_ = true;
        NETWORK.RequireFallingBlock(fallingIdx_, isFalling_);
    }
    else
    {
        int satelliteY = blocks_[Satellite]->GetY();
        if (satelliteY + Constants::Block::SIZE >= Constants::Board::HEIGHT)
        {
            blocks_[Standard]->SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
            blocks_[Satellite]->SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);

            canMove_ = false;
        }
    }
}


void GameGroupBlock::HandleRotation(float deltaTime) 
{
    if (!isRotating_)
    {
        return;
    }

    float fromX = blocks_[Standard]->GetX() + (Constants::Block::SIZE / 2.0f);
    float fromY = blocks_[Standard]->GetY() + (Constants::Block::SIZE / 2.0f);
    int direction = 0;

    float rotVelocity = deltaTime * Constants::Block::ROTATE_VELOCITY;
    rotateVelocity_ += rotVelocity;

    // 회전 상태별 처리
    bool rotationComplete = false;

    switch (rotateState_) 
    {
    case RotateState::Default:
        if (rotateVelocity_ >= 270.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(blockIndexX_);
            rotateVelocity_ = 270.0f;
            rotationComplete = true;
        }
        break;

    case RotateState::Right:
        if (rotateVelocity_ >= 360.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(blockIndexX_ + 1);
            rotateVelocity_ = 360.0f;
            rotationComplete = true;
            direction = 1;
        }
        break;

    case RotateState::Top:
        if (rotateVelocity_ >= 90.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(blockIndexX_);
            rotateVelocity_ = 90.0f;
            rotationComplete = true;
        }
        break;

    case RotateState::Left:
        if (rotateVelocity_ >= 180.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(blockIndexX_ - 1);
            rotateVelocity_ = 180.0f;
            rotationComplete = true;
            direction = -1;
        }
        break;
    }

    // 회전 위치 계산
    float x = fromX + (Constants::Block::SIZE * std::cos(GameUtils::ToRadians(rotateVelocity_)));
    float y = fromY + (Constants::Block::SIZE * -std::sin(GameUtils::ToRadians(rotateVelocity_)));

    x -= (Constants::Block::SIZE / 2.0f);
    y -= (Constants::Block::SIZE / 2.0f);

    if (rotationComplete) 
    {
        if (blocks_[Standard]->GetY() != y) 
        {
            y = blocks_[Standard]->GetY();
        }

        float finalX = GetPosXOfIdx(blockIndexX_);
        finalX += (direction * Constants::Block::SIZE);

        blocks_[Satellite]->SetPosition(finalX, y);

        if (auto stateManager = GAME_APP.GetStateManager().GetCurrentState())
        {
            if (auto gameState = dynamic_cast<GameState*>(stateManager.get()))
            {
                gameState->GetLocalPlayer()->UpdateTargetPosIdx();
            }
        }

        isRotating_ = false;
    }
    else
    {
        blocks_[Satellite]->SetPosition(x, y);
    }

    // 벽 또는 블록에 의한 회전으로 인한 기준 블록을 수평방향으로 이동시킬 때 처리
    if (isHorizontalMoving_) 
    {
        HandleHorizontalMovement(rotVelocity);
    }
}

void GameGroupBlock::HandleHorizontalMovement(float rotVelocity) 
{
    float movement = rotVelocity * Constants::Block::HORIZONTAL_VELOCITY;
    horizontalVelocity_ += movement;

    float posX = blocks_[Standard]->GetX();
    int direction = (rotateState_ == RotateState::Right) ? -1 : 1;

    posX += (movement * direction);

    if (!isRotating_ && horizontalVelocity_ >= Constants::Block::SIZE) 
    {
        blockIndexX_ += direction;
        float newPosX = GetPosXOfIdx(blockIndexX_);
        SetPosX(newPosX);

        if (auto stateManager = GAME_APP.GetStateManager().GetCurrentState())
        {
            if (auto gameState = dynamic_cast<GameState*>(stateManager.get()))
            {
                gameState->GetLocalPlayer()->UpdateTargetPosIdx();
            }
        }
        

        horizontalVelocity_ = 0.0f;
        isHorizontalMoving_ = false;
    }
    else 
    {
        blocks_[Standard]->SetX(posX);
    }
}

void GameGroupBlock::Update(float deltaTime) 
{
    if (state_ == BlockState::Playing || state_ == BlockState::Effecting) 
    {
        GroupBlock::Update(deltaTime);
    }

    if (state_ == BlockState::Playing && blocks_[Standard] && blocks_[Satellite]) 
    {
        // 회전 중인 경우 회전 처리
        if (isRotating_)
        {
            HandleRotation(deltaTime);
        }

        // 하강 처리
        float speed = deltaTime * Constants::Block::DOWN_VELOCITY * addVelocity_;
        velocity_ += speed;

        if (isRotating_) 
        {
            // 회전 중 하강
            blocks_[Standard]->SetY(blocks_[0]->GetY() + speed);
            blocks_[Satellite]->SetY(blocks_[1]->GetY() + speed);
        }
        else 
        {
            ForceVelocityY(velocity_);

            // 충돌 체크 및 네트워크 처리
            if ((NETWORK.IsRunning() && playerID_) || NETWORK.IsRunning() == false)
            {
                if (MoveDown() == false)
                {
                    SetState(BlockState::Effecting);
                    NETWORK.ChangeBlockState(static_cast<uint8_t>(BlockState::Effecting));
                }
            }
        }
    }
    else if (state_ == BlockState::Effecting) 
    {
        HandleEffectingState();
    }
}

void GameGroupBlock::HandleEffectingState() 
{
    if (!blocks_[Standard] || !blocks_[Satellite])
    {
        return;
    }

    if (blocks_[Standard]->GetState() == BlockState::Stationary && blocks_[Satellite]->GetState() == BlockState::Stationary)
    {
        UpdateBlockIndices();
        ResetVelocities();
        ProcessBlockPlacement();
    }
}

void GameGroupBlock::UpdateBlockIndices() 
{
    for (int i = 0; i < 2; ++i) 
    {
        if (!blocks_[i])
        {
            continue;
        }

        int xIdx = static_cast<int>(blocks_[i]->GetX() / Constants::Block::SIZE);
        int yIdx = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(blocks_[i]->GetY() / Constants::Block::SIZE);
        blocks_[i]->SetPosIdx(xIdx, yIdx);
    }
}

void GameGroupBlock::ResetVelocities() 
{
    velocity_ = 0.0f;
    addVelocity_ = 1.0f;
}

void GameGroupBlock::ProcessBlockPlacement() 
{

    if (NETWORK.IsRunning() && GAME_APP.GetPlayerManager().IsLocalPlayer(playerID_) == true)
    {
        SetState(BlockState::Stationary);
        NETWORK.ChangeBlockState(static_cast<uint8_t>(BlockState::Stationary));

        std::array<float, 2> pos1 = { blocks_[0]->GetX(),  blocks_[0]->GetY() };
        std::array<float, 2> pos2 = { blocks_[1]->GetX(),  blocks_[1]->GetY() };

        LOGGER.Info("ProcessBlockPlacement playerID_{} pos1 {} pos2 {}", playerID_, pos1, pos2);

        NETWORK.PushBlockInGame(pos1, pos2);

        if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            gameState->GetLocalPlayer()->PushBlockInGame(this);
        }
    }
}

void GameGroupBlock::GetCollisionRect(Block* block, SDL_Rect* rect, Constants::Direction dir) 
{
    if (!block || !rect)
    {
        return;
    }

    float halfWidth = block->GetWidth() / 2.0f;
    float halfHeight = block->GetHeight() / 2.0f;

    switch (dir) 
    {
    case Constants::Direction::Left:
        rect->x = block->GetX() - halfWidth;
        rect->y = block->GetY();
        rect->w = halfWidth;
        rect->h = block->GetHeight();
        break;

    case Constants::Direction::Top:
        rect->x = block->GetX();
        rect->y = block->GetY() - halfHeight;
        rect->w = block->GetWidth();
        rect->h = halfHeight;
        break;

    case Constants::Direction::Right:
        rect->x = block->GetX() + block->GetWidth();
        rect->y = block->GetY();
        rect->w = halfWidth;
        rect->h = block->GetHeight();
        break;

    case Constants::Direction::Bottom:
        rect->x = block->GetX();
        rect->y = block->GetY() + block->GetWidth();
        rect->w = block->GetWidth();
        rect->h = halfHeight;
        break;
    }
}

void GameGroupBlock::ResetBlock() 
{
    for (auto& block : blocks_) 
    {
        if (block != nullptr)
        {
            auto playerId = block->GetPlayerId();
            if (GAME_APP.GetPlayerManager().IsRemotePlayer(playerId) == true)
            {
                //LOGGER.Info("ResetBlock {}", playerId);
            }

            block = nullptr;
        }
    }

    isFalling_ = false;
    isRotating_ = false;
    checkingCollision_ = false;
    isHorizontalMoving_ = false;
    canMove_ = true;
    fallingIdx_ = -1;

    velocity_ = 0.0f;
    addVelocity_ = 1.0f;
    rotateVelocity_ = 0.0f;
    horizontalVelocity_ = 0.0f;

    SetState(BlockState::PlayOut);
}

void GameGroupBlock::ForceVelocityY(float velocity) 
{
    position_.y += velocity;

    if (isFalling_) 
    {
        blocks_[static_cast<size_t>(BlockIndex::Standard) + fallingIdx_]->SetY(position_.y);
    }
    else 
    {
        switch (rotateState_) 
        {
        case RotateState::Default:
            if (blocks_[Standard] && blocks_[Satellite])
            {
                blocks_[Standard]->SetY(position_.y);
                blocks_[Satellite]->SetY(position_.y + size_.y);
            }
            break;

        case RotateState::Left:
        case RotateState::Right:
            if (blocks_[Standard] && blocks_[Satellite])
            {
                blocks_[Standard]->SetY(position_.y);
                blocks_[Satellite]->SetY(position_.y);
            }
            break;

        case RotateState::Top:
            if (blocks_[Standard] && blocks_[Satellite])
            {
                blocks_[Standard]->SetY(position_.y);
                blocks_[Satellite]->SetY(position_.y - size_.y);
            }
            break;
        }
    }
}

void GameGroupBlock::ForceAddVelocityY(float velocity, bool send) 
{
    addVelocity_ += velocity;

   if (NETWORK.IsRunning() && send)
    {
       NETWORK.MoveBlock(static_cast<uint8_t>(Constants::Direction::Bottom), addVelocity_);
    }
}

void GameGroupBlock::SetEffectState(EffectState state) 
{
    if (blocks_[Standard] && blocks_[Satellite])
    {
        blocks_[Standard]->SetEffectState(state);
        blocks_[Satellite]->SetEffectState(state);
    }
}

void GameGroupBlock::SetGroupBlock(GroupBlock* block) 
{
    if (!block)
    {
        return;
    }

    ResetBlock();

    const auto sourceBlocks = block->GetBlocks();
    for (size_t i = 0; i < Constants::GroupBlock::COUNT; ++i)
    {
        if (sourceBlocks[i]) 
        {
            blocks_[i] = sourceBlocks[i]->Clone();
        }
    }

    position_ = block->GetPosition();
    size_ = block->GetSize();
    destination_rect_ = block->GetRect();
    groupBlockType_ = block->GetType();
    state_ = block->GetState();

    updateTime_ = SDL_GetTicks();
}

void GameGroupBlock::HandleSingleBlockFalling() 
{
    bool hasCollision = false;

    SDL_Rect resultRect;
    SDL_Rect controlRect;
    SDL_Rect targetRect;

    for (const auto& block : *gameBlockList_) 
    {
        if (!block)
        {
            continue;
        }

        RectUtils::ConvertFRectToRect(blocks_[fallingIdx_]->GetRect(), &controlRect);
        RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

        if (SDL_GetRectIntersection(&controlRect,&targetRect, &resultRect) == true)
        {
            hasCollision = true;
            break;
        }
    }

    if (hasCollision) 
    {
        blocks_[fallingIdx_]->SetY(resultRect.y - Constants::Block::SIZE);
        canMove_ = false;
        isFalling_ = false;
        NETWORK.RequireFallingBlock(fallingIdx_, isFalling_);
    }
    else 
    {
        float currentY = blocks_[fallingIdx_]->GetY();

        if (currentY + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
        {
            blocks_[fallingIdx_]->SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
            canMove_ = false;
            isFalling_ = false;
            NETWORK.RequireFallingBlock(fallingIdx_, isFalling_);
        }
    }
}

void GameGroupBlock::Rotate() 
{
    if (isRotating_ || isFalling_ || checkingCollision_) 
    {
        return;
    }

    switch (rotateState_) 
    {
    case RotateState::Default:
    case RotateState::Top:
        HandleDefaultTopRotation();
        break;

    case RotateState::Right:
        SetEnableRotState(RotateState::Top);
        break;

    case RotateState::Left:
        SetEnableRotState(RotateState::Default);
        break;
    }
}

void GameGroupBlock::HandleDefaultTopRotation() 
{
    SDL_Rect rightCollRect{}, leftCollRect{}, resultRect{}, targetRect{};
    bool rightColl = false, leftColl = false;

    GetCollisionRect(blocks_[static_cast<size_t>(BlockIndex::Standard)].get(), &leftCollRect, Constants::Direction::Left);
    GetCollisionRect(blocks_[static_cast<size_t>(BlockIndex::Standard)].get(), &rightCollRect, Constants::Direction::Right);

    for (const auto& block : *gameBlockList_) 
    {
        if (!block)
        {
            continue;
        }

        RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

        // 좌측 충돌 체크
        if (SDL_GetRectIntersection(&leftCollRect, &targetRect, &resultRect))
        {
            SDL_RectToFRect(&resultRect, &intersectResultRect_[0]);
            leftColl = true;
        }

        // 우측 충돌 체크
        if (SDL_GetRectIntersection(&rightCollRect, &targetRect, &resultRect))
        {
            SDL_RectToFRect(&resultRect, &intersectResultRect_[1]);
            rightColl = true;
        }

        if (rightColl && leftColl) 
        {
            return;
        }
    }

    // 게임 보드 경계 체크
    if (rightCollRect.x + Constants::Block::SIZE > Constants::Board::WIDTH - Constants::Board::WIDTH_MARGIN) 
    {
        rightColl = true;
    }

    if (leftCollRect.x < Constants::Board::WIDTH_MARGIN) 
    {
        leftColl = true;
    }

    if (rotateState_ == RotateState::Top) 
    {
        // 기준 블록 이동없이 회전 공간이 충분하면 회전 진행
        if ((leftColl == false && rightColl == false) ||
            (leftColl == false && rightColl == true)) 
        {
            SetEnableRotState(RotateState::Left);
        }
        // 회전 공간이 충분하지만 기준 블록이 이동해야 하는 경우
        else if (leftColl == true && rightColl == false) 
        {
            SetEnableRotState(RotateState::Left, true);
        }
    }
    else 
    {
        if ((leftColl == false && rightColl == false) ||
            (leftColl == true && rightColl == false)) 
        {
            SetEnableRotState(RotateState::Right);
        }
        else if (leftColl == false && rightColl == true) 
        {
            SetEnableRotState(RotateState::Right, true);
        }
    }
}

void GameGroupBlock::SetEnableRotState(RotateState state, bool horizontalMoving, bool enable, bool send) 
{
    if (enable) 
    {
        rotateState_ = state;
        isRotating_ = true;
        isHorizontalMoving_ = horizontalMoving;

        switch (state) {
        case RotateState::Default:
            rotateVelocity_ = 180.0f;
            break;
        case RotateState::Right:
            rotateVelocity_ = 270.0f;
            break;
        case RotateState::Top:
            rotateVelocity_ = 0.0f;
            break;
        case RotateState::Left:
            rotateVelocity_ = 90.0f;
            break;
        }

        if (send && NETWORK.IsRunning() && playerID_) 
        {
            NETWORK.RotateBlock(static_cast<uint8_t>(state), isHorizontalMoving_);
        }
    }
    else 
    {
        rotateState_ = state;
        isRotating_ = false;
        isHorizontalMoving_ = false;
    }
}

float GameGroupBlock::GetPosXOfIdx(int idx) const
{
    return ((idx * Constants::Block::SIZE) + Constants::Board::WIDTH_MARGIN);
}

int GameGroupBlock::CalculateIdxY(float y) const
{
    return (Constants::Board::BOARD_Y_COUNT- 2) - (int)(y / Constants::Block::SIZE);
}

void GameGroupBlock::SetPlayerID(uint8_t id)
{
    playerID_ = id;

    if (blocks_[Standard] && blocks_[Satellite])
    {
        blocks_[Standard]->SetPlayerID(playerID_);
        blocks_[Satellite]->SetPlayerID(playerID_);
    }
}

void GameGroupBlock::UpdateFallingBlock(uint8_t fallingIdx, bool falling)
{
    fallingIdx_ = fallingIdx;
    isFalling_ = falling;
}