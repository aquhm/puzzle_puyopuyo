#pragma once
/**
 *
 * 설명: 게임 보드 관련( 전체 블록 렌더링 및 이벤트 표시 )
 *
 */
#include <memory>
#include <list>
#include <array>

#include "../RenderableObject.hpp"
#include "../AnimatedObject.hpp"
#include "../../core/common/constants/Constants.hpp"

class ImageTexture;
class Block;
class GroupBlock;
class GameGroupBlock;

enum class BoardState 
{
    Normal,
    Attacking,
    Damaging,
    Lose
};

struct BlockTargetMark 
{
    uint8_t type{ 0 };
    float xPos{ 0 };
    float yPos{ 0 };

    SDL_FRect sourceRect
    {
        0, 0,
        Constants::Board::BLOCK_MARK_SIZE,
        Constants::Board::BLOCK_MARK_SIZE
    };
};

class GameBoard : public RenderableObject 
{
public:
    GameBoard() = default;
    ~GameBoard() override;

    GameBoard(const GameBoard&) = delete;
    GameBoard& operator=(const GameBoard&) = delete;
    GameBoard(GameBoard&&) noexcept = delete;
    GameBoard& operator=(GameBoard&&) noexcept = delete;

    bool Initialize(float xPos, float yPos, std::list<std::shared_ptr<Block>>& blockList, uint8_t playerId = 0);
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    void SetBlockInfoTexture(const std::shared_ptr<ImageTexture>& texture);
    void CreateNewBlockInGame(const std::shared_ptr<GroupBlock>& block);
    void ClearActiveGroupBlock() { active_group_block_ = nullptr; }
    void UpdateTargetBlockMark(const std::array<BlockTargetMark, 2>& markInfo);
    void ResetGroupBlock();

    void SetRenderTargetMark(bool render) { is_target_mark_ = render; }
    void SetState(BoardState state);
    [[nodiscard]] BoardState GetState() const { return state_; }


private:

    void UpdateNormalState(float deltaTime);
    void UpdateAttackingState(float deltaTime);
    void UpdateDamagingState(float deltaTime);
    void UpdateLosingState(float deltaTime);

    void ResetRenderTargetPosition();
    void InitializeRenderTarget();
    void UpdateRenderTarget();
    void RenderBackground();
    void InitializePositions(float xPos, float yPos);
    void RenderTargetMarks();
    void RenderFixedBlocks();

private:

    SDL_FRect background_source_rect_{};
    std::array<BlockTargetMark, 2> target_block_marks_{};

    std::shared_ptr<ImageTexture> source_block_;
    std::shared_ptr<ImageTexture> source_texture_;

    SDL_Texture* target_render_texture_{ nullptr };
    SDL_FRect target_render_rect_{};

    bool is_scaled_{ false };
    bool is_target_mark_{ false };
    uint8_t player_id_{ 0 };

    std::shared_ptr<GroupBlock> active_group_block_;
    std::list<std::shared_ptr<Block>>* block_list_{};

    BoardState state_{ BoardState::Normal };
    SDL_FlipMode flip_{ SDL_FLIP_NONE };

    // 애니메이션 관련 멤버
    float accumulate_time_{ 0.0f };
    bool is_rewind_{ false };
    float rotation_Accumulate_angle_{ 0.0f };
    float down_velocity_{ 0.0f };
    double angle_{ 0.0f };

    SDL_FPoint renderTargetPos_{};

};