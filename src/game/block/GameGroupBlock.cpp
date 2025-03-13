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
    , is_falling_(false)
    , is_rotating_(false)
    , checking_collision_(false)
    , can_move_(true)
    , falling_Index_(-1)
    , block_index_x_(0)
    , player_id_(0)
    , update_time_(0)
    , velocity_(0.0f)
    , add_velocity_(1.0f)
    , rotate_velocity_(0.0f)
    , horizontal_velocity_(0.0f)
    
{
}

GameGroupBlock::~GameGroupBlock() 
{
    //Release();
}

void GameGroupBlock::SetPosX(float x) 
{
    position_.x = x;
    block_index_x_ = static_cast<int>((position_.x - Constants::Board::WIDTH_MARGIN) / Constants::Block::SIZE);

    if (blocks_[0]) 
    {
        blocks_[0]->SetX(position_.x);
        blocks_[0]->SetPosIdx_X(block_index_x_);
    }

    if (blocks_[1]) 
    {
        switch (rotateState_) 
        {
        case RotateState::Default:
        case RotateState::Top:
            blocks_[1]->SetX(position_.x);
            blocks_[1]->SetPosIdx_X(block_index_x_);
            break;

        case RotateState::Right:
            blocks_[1]->SetX(position_.x + Constants::Block::SIZE);
            blocks_[1]->SetPosIdx_X(block_index_x_ + 1);
            break;

        case RotateState::Left:
            blocks_[1]->SetX(position_.x - Constants::Block::SIZE);
            blocks_[1]->SetPosIdx_X(block_index_x_ - 1);
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
    block_index_x_ = static_cast<int>((position_.x - Constants::Board::WIDTH_MARGIN) / Constants::Block::SIZE);
    blocks_[0]->SetPosIdx_X(block_index_x_);

    switch (rotateState_) {
    case RotateState::Default:
        blocks_[1]->SetPosition(position_.x, position_.y + Constants::Block::SIZE);
        blocks_[1]->SetPosIdx_X(block_index_x_);
        break;

    case RotateState::Top:
        blocks_[1]->SetPosition(position_.x, position_.y - Constants::Block::SIZE);
        blocks_[1]->SetPosIdx_X(block_index_x_);
        break;

    case RotateState::Right:
        blocks_[1]->SetPosition(position_.x + Constants::Block::SIZE, position_.y);
        blocks_[1]->SetPosIdx_X(block_index_x_ + 1);
        break;

    case RotateState::Left:
        blocks_[1]->SetPosition(position_.x - Constants::Block::SIZE, position_.y);
        blocks_[1]->SetPosIdx_X(block_index_x_ - 1);
        break;
    }

    UpdateDestRect();
}

void GameGroupBlock::MoveLeft(bool collisionCheck) 
{
    if (is_rotating_ || is_falling_ || checking_collision_ || !can_move_) 
    {
        return;
    }

    if (collisionCheck) 
    {
        
        SDL_Rect leftCollRects[2]{};
        SDL_Rect targetRect;

        GetCollisionRect(blocks_[Standard].get(), &leftCollRects[0], Constants::Direction::Left);
        GetCollisionRect(blocks_[Satellite].get(), &leftCollRects[1], Constants::Direction::Left);

        bool canMove = true;

        // 충돌 체크
        for (const auto& block : *game_block_list_) 
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
            if (auto& stateManager = GAME_APP.GetStateManager().GetCurrentState()) 
            {
                // GameState 타입으로 캐스팅 시도
                if (auto gameState = dynamic_cast<GameState*>(stateManager.get())) 
                {
                    if (block_index_x_ > 0 && gameState->GetLocalPlayer()->IsPossibleMove(block_index_x_ - 1)) 
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
    if (is_rotating_ || is_falling_ || checking_collision_ || !can_move_) 
    {
        return;
    }

    if (collisionCheck) 
    {   
        SDL_Rect rightCollRects[2]{};
        SDL_Rect targetRect;

        GetCollisionRect(blocks_[Standard].get(), &rightCollRects[0], Constants::Direction::Right);
        GetCollisionRect(blocks_[Satellite].get(), &rightCollRects[1], Constants::Direction::Right);

        bool canMove = true;

        for (const auto& block : *game_block_list_) 
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
            if (auto& stateManager = GAME_APP.GetStateManager().GetCurrentState())
            {
                // GameState 타입으로 캐스팅 시도
                if (auto gameState = dynamic_cast<GameState*>(stateManager.get()))
                {
                    if (block_index_x_ < Constants::Board::BOARD_X_COUNT - 1 && gameState->GetLocalPlayer()->IsPossibleMove(block_index_x_ + 1)) 
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
    checking_collision_ = true;    

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
        for (const auto& block : *game_block_list_) 
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
            can_move_ = false;
        }
        else if (satelliteY + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
        {
            blocks_[Standard]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE * 2));
            blocks_[Satellite]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE));
            can_move_ = false;
        }
        break;

    case RotateState::Right:
    case RotateState::Left:
        if (is_falling_ == false) 
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
        for (const auto& block : *game_block_list_) 
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
            can_move_ = false;
        }
        else if (satelliteY + Constants::Block::SIZE * 2 >= Constants::Board::HEIGHT) 
        {
            blocks_[Standard]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE));
            blocks_[Satellite]->SetY(static_cast<float>(Constants::Board::HEIGHT - Constants::Block::SIZE * 2));
            can_move_ = false;
        }
        break;
    }

    checking_collision_ = false;
    return can_move_;
}

void GameGroupBlock::HandleHorizontalCollision() 
{
    bool collision1 = false;
    bool collision2 = false;
    SDL_Rect resultRect[2]{};
    SDL_Rect controlRect[2]{};
    SDL_Rect targetRect[2]{};

    for (const auto& block : *game_block_list_) 
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
            SDL_RectToFRect(&resultRect[0], &intersect_result_rect_[0]);
            collision1 = true;
        }                

        if (SDL_GetRectIntersection(&controlRect[1], &targetRect[1], &resultRect[1]) == true)
        {
            SDL_RectToFRect(&resultRect[1], &intersect_result_rect_[1]);
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
        blocks_[Standard]->SetY(intersect_result_rect_[0].y - Constants::Block::SIZE);
        blocks_[Satellite]->SetY(intersect_result_rect_[1].y - Constants::Block::SIZE);
        can_move_ = false;
    }
    else if (collision1 == true && collision2 == false) 
    {
        blocks_[Standard]->SetY(intersect_result_rect_[0].y - Constants::Block::SIZE);
        falling_Index_ = Satellite;
        is_falling_ = true;
        NETWORK.RequireFallingBlock(falling_Index_, is_falling_);
    }
    else if (collision1 == false && collision2 == true) 
    {
        blocks_[Satellite]->SetY(intersect_result_rect_[1].y - Constants::Block::SIZE);
        falling_Index_ = Standard;
        is_falling_ = true;
        NETWORK.RequireFallingBlock(falling_Index_, is_falling_);
    }
    else
    {
        float satelliteY = blocks_[Satellite]->GetY();
        if (satelliteY + Constants::Block::SIZE >= Constants::Board::HEIGHT)
        {
            blocks_[Standard]->SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
            blocks_[Satellite]->SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);

            can_move_ = false;
        }
    }
}


void GameGroupBlock::HandleRotation(float deltaTime) 
{
    if (!is_rotating_)
    {
        return;
    }

    float fromX = blocks_[Standard]->GetX() + (Constants::Block::SIZE / 2.0f);
    float fromY = blocks_[Standard]->GetY() + (Constants::Block::SIZE / 2.0f);
    int direction = 0;

    float rotVelocity = deltaTime * Constants::Block::ROTATE_VELOCITY;
    rotate_velocity_ += rotVelocity;

    // 회전 상태별 처리
    bool rotationComplete = false;

    switch (rotateState_) 
    {
    case RotateState::Default:
        if (rotate_velocity_ >= 270.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(block_index_x_);
            rotate_velocity_ = 270.0f;
            rotationComplete = true;
        }
        break;

    case RotateState::Right:
        if (rotate_velocity_ >= 360.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(block_index_x_ + 1);
            rotate_velocity_ = 360.0f;
            rotationComplete = true;
            direction = 1;
        }
        break;

    case RotateState::Top:
        if (rotate_velocity_ >= 90.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(block_index_x_);
            rotate_velocity_ = 90.0f;
            rotationComplete = true;
        }
        break;

    case RotateState::Left:
        if (rotate_velocity_ >= 180.0f) 
        {
            blocks_[Satellite]->SetPosIdx_X(block_index_x_ - 1);
            rotate_velocity_ = 180.0f;
            rotationComplete = true;
            direction = -1;
        }
        break;
    }

    // 회전 위치 계산
    float x = fromX + (Constants::Block::SIZE * std::cos(GameUtils::ToRadians(rotate_velocity_)));
    float y = fromY + (Constants::Block::SIZE * -std::sin(GameUtils::ToRadians(rotate_velocity_)));

    x -= (Constants::Block::SIZE / 2.0f);
    y -= (Constants::Block::SIZE / 2.0f);

    if (rotationComplete) 
    {
        if (blocks_[Standard]->GetY() != y) 
        {
            y = blocks_[Standard]->GetY();
        }

        float finalX = GetPosXOfIdx(block_index_x_);
        finalX += (direction * Constants::Block::SIZE);

        blocks_[Satellite]->SetPosition(finalX, y);

        if (auto& stateManager = GAME_APP.GetStateManager().GetCurrentState())
        {
            if (auto gameState = dynamic_cast<GameState*>(stateManager.get()))
            {
                gameState->GetLocalPlayer()->UpdateTargetPosIdx();
            }
        }

        is_rotating_ = false;
    }
    else
    {
        blocks_[Satellite]->SetPosition(x, y);
    }

    // 벽 또는 블록에 의한 회전으로 인한 기준 블록을 수평방향으로 이동시킬 때 처리
    if (is_horizontal_moving_) 
    {
        HandleHorizontalMovement(rotVelocity);
    }
}

void GameGroupBlock::HandleHorizontalMovement(float rotVelocity) 
{
    float movement = rotVelocity * Constants::Block::HORIZONTAL_VELOCITY;
    horizontal_velocity_ += movement;

    float posX = blocks_[Standard]->GetX();
    int direction = (rotateState_ == RotateState::Right) ? -1 : 1;

    posX += (movement * direction);

    if (!is_rotating_ && horizontal_velocity_ >= Constants::Block::SIZE) 
    {
        block_index_x_ += direction;
        float newPosX = GetPosXOfIdx(block_index_x_);
        SetPosX(newPosX);

        if (auto& stateManager = GAME_APP.GetStateManager().GetCurrentState())
        {
            if (auto gameState = dynamic_cast<GameState*>(stateManager.get()))
            {
                gameState->GetLocalPlayer()->UpdateTargetPosIdx();
            }
        }
        

        horizontal_velocity_ = 0.0f;
        is_horizontal_moving_ = false;
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

    /*if (player_id_ == 1)
    {
        LOGGER.Info("GameGroupBlock::Update state_ = {} velocity_ = {} y = {}", (int)state_, velocity_, position_.y);
    }*/

    if (state_ == BlockState::Playing && blocks_[Standard] && blocks_[Satellite]) 
    {
        // 회전 중인 경우 회전 처리
        if (is_rotating_)
        {
            HandleRotation(deltaTime);
        }

        // 하강 처리
        float speed = deltaTime * Constants::Block::DOWN_VELOCITY * add_velocity_;
        velocity_ += speed;

        if (is_rotating_) 
        {
            // 회전 중 하강
            blocks_[Standard]->SetY(blocks_[0]->GetY() + speed);
            blocks_[Satellite]->SetY(blocks_[1]->GetY() + speed);
        }
        else 
        {
            ForceVelocityY(velocity_);

            // 충돌 체크 및 네트워크 처리
            if (NETWORK.IsRunning() && GAME_APP.GetPlayerManager().IsLocalPlayer(player_id_) == true)
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
    add_velocity_ = 1.0f;
}

void GameGroupBlock::ProcessBlockPlacement() 
{
    if (!blocks_[Standard] || !blocks_[Satellite])
    {
        return;
    }

    if (NETWORK.IsRunning() && GAME_APP.GetPlayerManager().IsLocalPlayer(player_id_) == true)
    {
        SetState(BlockState::Stationary);
        NETWORK.ChangeBlockState(static_cast<uint8_t>(BlockState::Stationary));

        std::array<float, 2> pos1 = { blocks_[0]->GetX(),  blocks_[0]->GetY() };
        std::array<float, 2> pos2 = { blocks_[1]->GetX(),  blocks_[1]->GetY() };

        //LOGGER.Info("ProcessBlockPlacement player_id_{} pos1 {} pos2 {}", player_id_, pos1, pos2);

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
        rect->x = static_cast<int>(block->GetX() - halfWidth);
        rect->y = static_cast<int>(block->GetY());
        rect->w = static_cast<int>(halfWidth);
        rect->h = static_cast<int>(block->GetHeight());
        break;

    case Constants::Direction::Top:
        rect->x = static_cast<int>(block->GetX());
        rect->y = static_cast<int>(block->GetY() - halfHeight);
        rect->w = static_cast<int>(block->GetWidth());
        rect->h = static_cast<int>(halfHeight);
        break;

    case Constants::Direction::Right:
        rect->x = static_cast<int>(block->GetX() + block->GetWidth());
        rect->y = static_cast<int>(block->GetY());
        rect->w = static_cast<int>(halfWidth);
        rect->h = static_cast<int>(block->GetHeight());
        break;

    case Constants::Direction::Bottom:
        rect->x = static_cast<int>(block->GetX());
        rect->y = static_cast<int>(block->GetY() + block->GetWidth());
        rect->w = static_cast<int>(block->GetWidth());
        rect->h = static_cast<int>(halfHeight);
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

            //block->Release();
            block = nullptr;
        }
    }

    is_falling_ = false;
    is_rotating_ = false;
    checking_collision_ = false;
    is_horizontal_moving_ = false;
    can_move_ = true;
    falling_Index_ = -1;

    velocity_ = 0.0f;
    add_velocity_ = 1.0f;
    rotate_velocity_ = 0.0f;
    horizontal_velocity_ = 0.0f;

    SetState(BlockState::PlayOut);
}

void GameGroupBlock::ForceVelocityY(float velocity) 
{
    position_.y += velocity;

    if (is_falling_) 
    {
        blocks_[static_cast<size_t>(BlockIndex::Standard) + falling_Index_]->SetY(position_.y);
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
    add_velocity_ += velocity;

   if (NETWORK.IsRunning() && send)
    {
       NETWORK.MoveBlock(static_cast<uint8_t>(Constants::Direction::Bottom), add_velocity_);
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
    SetState(BlockState::Playing);

    const auto sourceBlocks = block->GetBlocks();
    for (size_t i = 0; i < Constants::GroupBlock::COUNT; ++i)
    {
        if (sourceBlocks[i]) 
        {
            blocks_[i] = sourceBlocks[i]->Clone();
            blocks_[i]->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
        }
    }

    position_ = block->GetPosition();
    size_ = block->GetSize();
    destination_rect_ = block->GetRect();
    group_block_type_ = block->GetType();
    state_ = block->GetState();

    update_time_ = SDL_GetTicks();
}

void GameGroupBlock::HandleSingleBlockFalling() 
{
    bool hasCollision = false;

    SDL_Rect resultRect;
    SDL_Rect controlRect;
    SDL_Rect targetRect;

    for (const auto& block : *game_block_list_) 
    {
        if (!block)
        {
            continue;
        }

        RectUtils::ConvertFRectToRect(blocks_[falling_Index_]->GetRect(), &controlRect);
        RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

        if (SDL_GetRectIntersection(&controlRect,&targetRect, &resultRect) == true)
        {
            hasCollision = true;
            break;
        }
    }

    if (hasCollision) 
    {
        blocks_[falling_Index_]->SetY(resultRect.y - Constants::Block::SIZE);
        can_move_ = false;
        is_falling_ = false;
        NETWORK.RequireFallingBlock(falling_Index_, is_falling_);
    }
    else 
    {
        float currentY = blocks_[falling_Index_]->GetY();

        if (currentY + Constants::Block::SIZE >= Constants::Board::HEIGHT) 
        {
            blocks_[falling_Index_]->SetY(Constants::Board::HEIGHT - Constants::Block::SIZE);
            can_move_ = false;
            is_falling_ = false;
            NETWORK.RequireFallingBlock(falling_Index_, is_falling_);
        }
    }
}

void GameGroupBlock::Rotate() 
{
    if (is_rotating_ || is_falling_ || checking_collision_) 
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

    for (const auto& block : *game_block_list_) 
    {
        if (!block)
        {
            continue;
        }

        RectUtils::ConvertFRectToRect(block->GetRect(), &targetRect);

        // 좌측 충돌 체크
        if (SDL_GetRectIntersection(&leftCollRect, &targetRect, &resultRect))
        {
            SDL_RectToFRect(&resultRect, &intersect_result_rect_[0]);
            leftColl = true;
        }

        // 우측 충돌 체크
        if (SDL_GetRectIntersection(&rightCollRect, &targetRect, &resultRect))
        {
            SDL_RectToFRect(&resultRect, &intersect_result_rect_[1]);
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
        is_rotating_ = true;
        is_horizontal_moving_ = horizontalMoving;

        switch (state) {
        case RotateState::Default:
            rotate_velocity_ = 180.0f;
            break;
        case RotateState::Right:
            rotate_velocity_ = 270.0f;
            break;
        case RotateState::Top:
            rotate_velocity_ = 0.0f;
            break;
        case RotateState::Left:
            rotate_velocity_ = 90.0f;
            break;
        }

        if (send && NETWORK.IsRunning() && player_id_) 
        {
            NETWORK.RotateBlock(static_cast<uint8_t>(state), is_horizontal_moving_);
        }
    }
    else 
    {
        rotateState_ = state;
        is_rotating_ = false;
        is_horizontal_moving_ = false;
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
    player_id_ = id;

    if (blocks_[Standard] && blocks_[Satellite])
    {
        blocks_[Standard]->SetPlayerID(player_id_);
        blocks_[Satellite]->SetPlayerID(player_id_);
    }
}

void GameGroupBlock::UpdateFallingBlock(uint8_t fallingIdx, bool falling)
{
    falling_Index_ = fallingIdx;
    is_falling_ = falling;
}

void GameGroupBlock::Release()
{
    ResetBlock();
    GroupBlock::Release();
}