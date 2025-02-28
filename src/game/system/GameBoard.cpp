#include "GameBoard.hpp"


#include "../block/GroupBlock.hpp"
#include "../block/GameGroupBlock.hpp"

#include "../../core/GameApp.hpp"
#include "../../core/GameUtils.hpp"
#include "../../core/manager/ParticleManager.hpp"

#include "../../texture/ImageTexture.hpp"
#include "../../states/GameState.hpp"
#include "../../network/NetworkController.hpp"

#include "../../utils/Logger.hpp"

#include <algorithm>
#include <functional>
#include <cassert>



GameBoard::~GameBoard() 
{
    Release();
}

bool GameBoard::Initialize(float xPos, float yPos, std::list<std::shared_ptr<Block>>& blockList, uint8_t playerId)
{
    try
    {
        blockList_ = &blockList;
        playerID_ = playerId;

        // 배경 텍스처 로드
        sourceBlock_ = ImageTexture::Create("FIELD/BG_00.png");
        if (!sourceBlock_)
        {
            throw std::runtime_error("Failed to load background texture");
        }

        newBlockPosition_ = std::make_unique<AnimatedObject>();
        if (!newBlockPosition_)
        {
            throw std::runtime_error("Failed to create animation object");
        }

        InitializeRenderTarget();
        InitializePositions(xPos, yPos);

        // 새 블록 위치 설정
        newBlockPosition_->SetPosition(Constants::Board::NEW_BLOCK_POS_X, Constants::Board::NEW_BLOCK_POS_Y);

        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "GameBoard initialization failed: %s", e.what());
        return false;
    }
}

void GameBoard::InitializeRenderTarget() 
{
    if (!targetRenderTexture_) 
    {
        targetRenderTexture_ = SDL_CreateTexture(
            GAME_APP.GetRenderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            Constants::Board::WIDTH,
            Constants::Board::HEIGHT
        );

        if (!targetRenderTexture_) 
        {
            throw std::runtime_error(std::string("Failed to create render texture: ") + SDL_GetError());
        }

        SDL_SetTextureBlendMode(targetRenderTexture_, SDL_BLENDMODE_BLEND);
    }
}

void GameBoard::InitializePositions(float xPos, float yPos)
{
    renderTargetPos_ = { xPos, yPos };

    // 렌더 타겟 rect 초기화
    targetRenderRect_ = {
        xPos,
        yPos,
        Constants::Board::WIDTH,
        Constants::Board::HEIGHT
    };

    // 배경 소스 rect 초기화
    backgroundSourceRect_ = {
        Constants::Board::WIDTH_MARGIN,
        0,
        Constants::Board::WIDTH,
        Constants::Board::HEIGHT - static_cast<int>(Constants::Block::SIZE)
    };

    SetPosition(0, 0);
    SetScale(Constants::Board::WIDTH, Constants::Board::HEIGHT);
}

void GameBoard::SetBlockInfoTexture(const std::shared_ptr<ImageTexture>& texture) 
{
    if (newBlockPosition_) 
    {
        SDL_FRect srcRect = { 225, 384, 160, 30 };
        SDL_FRect viewRect = { 0, 0, 31, 30 };

        newBlockPosition_->SetTextureInfo(
            texture,
            viewRect,
            srcRect,
            {1,0}            
        );

        newBlockPosition_->SetFrameTime(Constants::Board::NEW_BLOCK_ANIM_SPEED);
        newBlockPosition_->Play();
    }

    puyoSourceTexture_ = texture;
}

void GameBoard::CreateNewBlockInGame(const std::shared_ptr<GroupBlock>& block) 
{
    if (activeGroupBlock_ && activeGroupBlock_->GetState() == BlockState::Playing) 
    {
        return;
    }

    isTargetMark_ = true;
    activeGroupBlock_ = block;

    // 새 블록 위치 설정
    activeGroupBlock_->SetPosXY(Constants::Board::WIDTH_MARGIN + Constants::Block::SIZE * 2, -Constants::Block::SIZE * 2);

    // 블록 인덱스 업데이트
    if (auto blocks = activeGroupBlock_->GetBlocks(); blocks.size() > 0)
    {
        for (int i = 0; i < 2; ++i) 
        {
            if (blocks[i]) 
            {
                int xIdx = static_cast<int>(blocks[i]->GetX() / Constants::Block::SIZE);
                blocks[i]->SetPosIdx_X(xIdx);
            }
        }
    }
}

// GameBoard.cpp
void GameBoard::UpdateNormalState(float deltaTime) 
{
}

void GameBoard::UpdateAttackingState(float deltaTime) 
{
    const float speed = deltaTime * Constants::Board::ATTACK_SPEED;

    if (!isRewind_) 
    {
        if (targetRenderRect_.x < renderTargetPos_.x + 30) 
        {
            // 전진 모션
            targetRenderRect_.x += speed;
            targetRenderRect_.y -= speed;
            targetRenderRect_.w -= speed * 2;
            targetRenderRect_.h += speed;
        }
        else 
        {
            isRewind_ = true;
        }
    }
    else 
    {
        // 후퇴 모션
        targetRenderRect_.x -= speed;
        targetRenderRect_.y += speed;
        targetRenderRect_.w += speed * 2;
        targetRenderRect_.h -= speed;

        if (targetRenderRect_.x <= renderTargetPos_.x) 
        {
            // 원위치로 복귀
            ResetRenderTargetPosition();
            SetState(BoardState::Normal);
        }
    }

    accumTime_ += deltaTime;
}

void GameBoard::UpdateDamagingState(float deltaTime) 
{
    const float rotateSpeed = deltaTime * Constants::Board::ROTATE_SPEED;
    rotAccumAngle_ += rotateSpeed;

    if (rotAccumAngle_ < Constants::Math::CIRCLE_ANGLE * 4) 
    {
        targetRenderRect_.x = renderTargetPos_.x + Constants::Board::CURVE_SPEED * std::sin(GameUtils::ToRadians(rotAccumAngle_));
    }
    else 
    {
        targetRenderRect_.x = renderTargetPos_.x;
        SetState(BoardState::Normal);
    }
}

void GameBoard::UpdateLosingState(float deltaTime) 
{
    constexpr float MAX_TILT_ANGLE = -35.0f;
    constexpr float TILT_SPEED = 15.0f;
    constexpr float FALL_SPEED = 370.0f;

    if (angle_ > MAX_TILT_ANGLE) 
    {
        angle_ -= deltaTime * TILT_SPEED;
    }

    if (targetRenderRect_.y < GAME_APP.GetWindowHeight() + 50) 
    {
        targetRenderRect_.y += deltaTime * FALL_SPEED;
    }
}

void GameBoard::Update(float deltaTime) 
{
    if (newBlockPosition_) 
    {
        newBlockPosition_->Update(deltaTime);
    }

    switch (state_) 
    {
    case BoardState::Normal:
        UpdateNormalState(deltaTime);
        break;
    case BoardState::Attacking:
        UpdateAttackingState(deltaTime);
        break;
    case BoardState::Damaging:
        UpdateDamagingState(deltaTime);
        break;
    case BoardState::Lose:
        UpdateLosingState(deltaTime);
        break;
    }
}

void GameBoard::ResetRenderTargetPosition() 
{
    targetRenderRect_.x = renderTargetPos_.x;
    targetRenderRect_.y = renderTargetPos_.y;
    targetRenderRect_.w = Constants::Board::WIDTH;
    targetRenderRect_.h = Constants::Board::HEIGHT;
}

void GameBoard::SetState(BoardState newState) 
{
    state_ = newState;
    ResetRenderTargetPosition();

    switch (state_) {
    case BoardState::Normal:
        isRewind_ = false;
        rotAccumAngle_ = 0.0f;
        angle_ = 0.0f;
        break;

    case BoardState::Attacking:
        isRewind_ = false;
        break;

    case BoardState::Damaging:
        rotAccumAngle_ = 0.0f;
        break;

    case BoardState::Lose:
        break;
    }

    accumTime_ = 0.0f;
}

// GameBoard.cpp

void GameBoard::Render() 
{
    if (!is_visible_ || !targetRenderTexture_) 
    {
        return;
    }

    if (state_ == BoardState::Lose && targetRenderRect_.y >= GAME_APP.GetWindowHeight() + 50) 
    {
        return;
    }

    // 렌더 타겟 설정
    SDL_SetRenderTarget(GAME_APP.GetRenderer(), targetRenderTexture_);

    // 배경 렌더링
    RenderBackground();

    // 새로운 블록 위치 표시 렌더링
    RenderNewBlockPosition();

    // 타겟 마크 렌더링
    RenderTargetMarks();

    // 활성 그룹 블록 렌더링
    if (activeGroupBlock_) 
    {
        activeGroupBlock_->Render();
    }

    // 고정된 블록들 렌더링
    RenderFixedBlocks();

    // 파티클 렌더링     
    GAME_APP.GetParticleManager().RenderForPlayer(playerID_);

    // 렌더 타겟 복원 및 최종 렌더링
    SDL_SetRenderTarget(GAME_APP.GetRenderer(), nullptr);

    SDL_RenderTextureRotated(
        GAME_APP.GetRenderer(),
        targetRenderTexture_,
        nullptr,
        &targetRenderRect_,
        angle_,
        nullptr,
        flip_
    );
}

void GameBoard::RenderBackground() 
{
    if (sourceBlock_) 
    {
        sourceBlock_->RenderScaled(&backgroundSourceRect_, &destination_rect_);
    }
}

void GameBoard::RenderNewBlockPosition() 
{
    if (newBlockPosition_) 
    {
        // 첫 번째 블록 위치
        newBlockPosition_->SetPosition(Constants::Board::NEW_BLOCK_POS_X, Constants::Board::NEW_BLOCK_POS_Y);
        newBlockPosition_->Render();

        // 두 번째 블록 위치
        newBlockPosition_->SetPosition(Constants::Board::NEW_BLOCK_POS_X + Constants::Block::SIZE, Constants::Board::NEW_BLOCK_POS_Y);
        newBlockPosition_->Render();
    }
}

void GameBoard::RenderTargetMarks() 
{
    if (puyoSourceTexture_ && isTargetMark_) 
    {
        for (const auto& mark : targetBlockMarks_) 
        {
            puyoSourceTexture_->Render(mark.xPos, mark.yPos, &mark.sourceRect);
        }
    }
}

void GameBoard::RenderFixedBlocks() 
{
    if (blockList_->empty() == false) 
    {
        std::ranges::for_each(*blockList_, std::mem_fn(&Block::Render));
    }
}

void GameBoard::Release() 
{
    try 
    {
        sourceBlock_.reset();
        newBlockPosition_.reset();

        if (targetRenderTexture_) 
        {
            SDL_DestroyTexture(targetRenderTexture_);
            targetRenderTexture_ = nullptr;
        }

        // 기타 멤버 변수 초기화
        blockList_->clear();
        activeGroupBlock_.reset();
        puyoSourceTexture_.reset();

        // 상태 관련 변수 초기화
        state_ = BoardState::Normal;
        isTargetMark_ = false;
        accumTime_ = 0.0f;
        angle_ = 0.0f;

    }
    catch (const std::exception& e)
    {
        SDL_Log("Error during GameBoard release: %s", e.what());
    }
}

void GameBoard::ResetGroupBlock()
{
    activeGroupBlock_.reset();
}

void GameBoard::UpdateTargetBlockMark(const std::array<BlockTargetMark, 2>& markInfo)
{
    if (markInfo.empty()) 
    {
        return;
    }

    targetBlockMarks_ = markInfo;
    isTargetMark_ = true;

    // 각 마크의 소스 렉트 업데이트
    for (int i = 0; i < 2; ++i) 
    {
        auto& mark = targetBlockMarks_[i];

        switch (static_cast<BlockType>(mark.type)) 
        {
        case BlockType::Yellow:
            mark.sourceRect = {
                Constants::Board::BLOCK_MARK_POS_X,
                Constants::Board::BLOCK_MARK_POS_Y,
                Constants::Board::BLOCK_MARK_SIZE,
                Constants::Board::BLOCK_MARK_SIZE
            };
            break;

        case BlockType::Red:
            mark.sourceRect = {
                Constants::Board::BLOCK_MARK_POS_X + Constants::Board::BLOCK_MARK_SIZE + 1,
                Constants::Board::BLOCK_MARK_POS_Y,
                Constants::Board::BLOCK_MARK_SIZE,
                Constants::Board::BLOCK_MARK_SIZE
            };
            break;

        case BlockType::Purple:
            mark.sourceRect = {
                Constants::Board::BLOCK_MARK_POS_X,
                Constants::Board::BLOCK_MARK_POS_Y + Constants::Board::BLOCK_MARK_SIZE + 1,
                Constants::Board::BLOCK_MARK_SIZE,
                Constants::Board::BLOCK_MARK_SIZE
            };
            break;

        case BlockType::Green:
            mark.sourceRect = {
                Constants::Board::BLOCK_MARK_POS_X + Constants::Board::BLOCK_MARK_SIZE + 1,
                Constants::Board::BLOCK_MARK_POS_Y + Constants::Board::BLOCK_MARK_SIZE + 1,
                Constants::Board::BLOCK_MARK_SIZE,
                Constants::Board::BLOCK_MARK_SIZE
            };
            break;

        case BlockType::Blue:
            mark.sourceRect = {
                Constants::Board::BLOCK_MARK_POS_X + Constants::Board::BLOCK_MARK_SIZE + 1,
                Constants::Board::BLOCK_MARK_POS_Y + (Constants::Board::BLOCK_MARK_SIZE + 1) * 2,
                Constants::Board::BLOCK_MARK_SIZE,
                Constants::Board::BLOCK_MARK_SIZE
            };
            break;
        }
    }
}

void GameBoard::UpdateRenderTarget() 
{
    if (!targetRenderTexture_) 
    {
        return;
    }

    // 렌더 타겟 크기 업데이트
    SDL_FPoint size;
    SDL_GetTextureSize(targetRenderTexture_, &size.x, &size.y);

    // 렌더 타겟 위치 및 크기 업데이트
    targetRenderRect_.x = renderTargetPos_.x;
    targetRenderRect_.y = renderTargetPos_.y;
    targetRenderRect_.w = size.x;
    targetRenderRect_.h = size.y;

    // 블렌드 모드 설정
    SDL_SetTextureBlendMode(targetRenderTexture_, SDL_BLENDMODE_BLEND);
}