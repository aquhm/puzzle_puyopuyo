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
        block_list_ = &blockList;
        player_id_ = playerId;

        source_block_ = ImageTexture::Create("FIELD/BG_00.png");
        if (!source_block_)
        {
            throw std::runtime_error("Failed to load background texture");
        }

        InitializeRenderTarget();
        InitializePositions(xPos, yPos);

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
    if (!target_render_texture_) 
    {
        target_render_texture_ = SDL_CreateTexture(
            GAME_APP.GetRenderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            Constants::Board::WIDTH,
            Constants::Board::HEIGHT
        );

        if (!target_render_texture_) 
        {
            throw std::runtime_error(std::string("Failed to create render texture: ") + SDL_GetError());
        }

        SDL_SetTextureBlendMode(target_render_texture_, SDL_BLENDMODE_BLEND);
    }
}

void GameBoard::InitializePositions(float xPos, float yPos)
{
    renderTargetPos_ = { xPos, yPos };

    // 렌더 타겟 rect 초기화
    target_render_rect_ = {
        xPos,
        yPos,
        Constants::Board::WIDTH,
        Constants::Board::HEIGHT
    };

    // 배경 소스 rect 초기화
    background_source_rect_ = {
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
    source_texture_ = texture;
}

void GameBoard::CreateNewBlockInGame(const std::shared_ptr<GroupBlock>& block) 
{
    if (active_group_block_ && active_group_block_->GetState() == BlockState::Playing) 
    {
        return;
    }

    is_target_mark_ = true;
    active_group_block_ = block;

    // 새 블록 위치 설정
    active_group_block_->SetPosXY(Constants::Board::WIDTH_MARGIN + Constants::Block::SIZE * 2, -Constants::Block::SIZE * 2);

    // 블록 인덱스 업데이트
    if (auto blocks = active_group_block_->GetBlocks(); blocks.size() > 0)
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

void GameBoard::UpdateNormalState(float deltaTime) 
{
}

void GameBoard::UpdateAttackingState(float deltaTime) 
{
    const float speed = deltaTime * Constants::Board::ATTACK_SPEED;

    if (!is_rewind_) 
    {
        if (target_render_rect_.x < renderTargetPos_.x + 30) 
        {
            // 전진 모션
            target_render_rect_.x += speed;
            target_render_rect_.y -= speed;
            target_render_rect_.w -= speed * 2;
            target_render_rect_.h += speed;
        }
        else 
        {
            is_rewind_ = true;
        }
    }
    else 
    {
        // 후퇴 모션
        target_render_rect_.x -= speed;
        target_render_rect_.y += speed;
        target_render_rect_.w += speed * 2;
        target_render_rect_.h -= speed;

        if (target_render_rect_.x <= renderTargetPos_.x) 
        {
            // 원위치로 복귀
            ResetRenderTargetPosition();
            SetState(BoardState::Normal);
        }
    }

    accumulate_time_ += deltaTime;
}

void GameBoard::UpdateDamagingState(float deltaTime) 
{
    const float rotateSpeed = deltaTime * Constants::Board::ROTATE_SPEED;
    rotation_Accumulate_angle_ += rotateSpeed;

    if (rotation_Accumulate_angle_ < Constants::Math::CIRCLE_ANGLE * 4) 
    {
        target_render_rect_.x = renderTargetPos_.x + Constants::Board::CURVE_SPEED * std::sin(GameUtils::ToRadians(rotation_Accumulate_angle_));
    }
    else 
    {
        target_render_rect_.x = renderTargetPos_.x;
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

    if (target_render_rect_.y < GAME_APP.GetWindowHeight() + 50) 
    {
        target_render_rect_.y += deltaTime * FALL_SPEED;
    }
}

void GameBoard::Update(float deltaTime) 
{
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
    target_render_rect_.x = renderTargetPos_.x;
    target_render_rect_.y = renderTargetPos_.y;
    target_render_rect_.w = Constants::Board::WIDTH;
    target_render_rect_.h = Constants::Board::HEIGHT;
}

void GameBoard::SetState(BoardState newState) 
{
    state_ = newState;
    ResetRenderTargetPosition();

    switch (state_) {
    case BoardState::Normal:
        is_rewind_ = false;
        rotation_Accumulate_angle_ = 0.0f;
        angle_ = 0.0f;
        break;

    case BoardState::Attacking:
        is_rewind_ = false;
        break;

    case BoardState::Damaging:
        rotation_Accumulate_angle_ = 0.0f;
        break;

    case BoardState::Lose:
        break;
    }

    accumulate_time_ = 0.0f;
}

void GameBoard::Render() 
{
    if (!is_visible_ || !target_render_texture_) 
    {
        return;
    }

    if (state_ == BoardState::Lose && target_render_rect_.y >= GAME_APP.GetWindowHeight() + 50) 
    {
        return;
    }

    // 렌더 타겟 설정
    SDL_SetRenderTarget(GAME_APP.GetRenderer(), target_render_texture_);

    // 배경 렌더링
    RenderBackground();

    // 타겟 마크 렌더링
    RenderTargetMarks();

    // 활성 그룹 블록 렌더링
    if (active_group_block_) 
    {
        active_group_block_->Render();
    }

    RenderFixedBlocks();

    GAME_APP.GetParticleManager().RenderForPlayer(player_id_);

    SDL_SetRenderTarget(GAME_APP.GetRenderer(), nullptr);

    SDL_RenderTextureRotated(
        GAME_APP.GetRenderer(),
        target_render_texture_,
        nullptr,
        &target_render_rect_,
        angle_,
        nullptr,
        flip_
    );
}

void GameBoard::RenderBackground() 
{
    if (source_block_) 
    {
        source_block_->RenderScaled(&background_source_rect_, &destination_rect_);
    }
}

void GameBoard::RenderTargetMarks() 
{
    if (source_texture_ && is_target_mark_) 
    {
        for (const auto& mark : target_block_marks_) 
        {
            source_texture_->Render(mark.xPos, mark.yPos, &mark.sourceRect);
        }
    }
}

void GameBoard::RenderFixedBlocks() 
{
    if (block_list_->empty() == false) 
    {
        std::ranges::for_each(*block_list_, std::mem_fn(&Block::Render));
    }
}

void GameBoard::Release() 
{
    try 
    {
        source_block_.reset();

        if (target_render_texture_) 
        {
            SDL_DestroyTexture(target_render_texture_);
            target_render_texture_ = nullptr;
        }

        block_list_->clear();
        active_group_block_.reset();
        source_texture_.reset();

        state_ = BoardState::Normal;
        is_target_mark_ = false;
        accumulate_time_ = 0.0f;
        angle_ = 0.0f;

    }
    catch (const std::exception& e)
    {
        SDL_Log("Error during GameBoard release: %s", e.what());
    }
}

void GameBoard::ResetGroupBlock()
{
    active_group_block_.reset();
}

void GameBoard::UpdateTargetBlockMark(const std::array<BlockTargetMark, 2>& markInfo)
{
    if (markInfo.empty()) 
    {
        return;
    }

    target_block_marks_ = markInfo;
    is_target_mark_ = true;

    // 각 마크의 소스 렉트 업데이트
    for (int i = 0; i < 2; ++i) 
    {
        auto& mark = target_block_marks_[i];

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
    if (!target_render_texture_) 
    {
        return;
    }

    // 렌더 타겟 크기 업데이트
    SDL_FPoint size;
    SDL_GetTextureSize(target_render_texture_, &size.x, &size.y);

    // 렌더 타겟 위치 및 크기 업데이트
    target_render_rect_.x = renderTargetPos_.x;
    target_render_rect_.y = renderTargetPos_.y;
    target_render_rect_.w = size.x;
    target_render_rect_.h = size.y;

    // 블렌드 모드 설정
    SDL_SetTextureBlendMode(target_render_texture_, SDL_BLENDMODE_BLEND);
}