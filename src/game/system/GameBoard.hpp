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
    void ClearActiveGroupBlock() { activeGroupBlock_ = nullptr; }
    void UpdateTargetBlockMark(const std::array<BlockTargetMark, 2>& markInfo);
    void ResetGroupBlock();

    void SetRenderTargetMark(bool render) { isTargetMark_ = render; }
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
    void RenderNewBlockPosition();
    void RenderTargetMarks();
    void RenderFixedBlocks();

private:

    SDL_FRect backgroundSourceRect_{};
    std::array<BlockTargetMark, 2> targetBlockMarks_{};

    std::shared_ptr<ImageTexture> sourceBlock_;
    std::unique_ptr<AnimatedObject> newBlockPosition_;
    std::shared_ptr<ImageTexture> puyoSourceTexture_;

    SDL_Texture* targetRenderTexture_{ nullptr };
    SDL_FRect targetRenderRect_{};

    bool isScaled_{ false };
    bool isTargetMark_{ false };
    uint8_t playerID_{ 0 };

    std::shared_ptr<GroupBlock> activeGroupBlock_;
    std::list<std::shared_ptr<Block>>* blockList_;

    BoardState state_{ BoardState::Normal };
    SDL_FlipMode flip_{ SDL_FLIP_NONE };

    // 애니메이션 관련 멤버
    float accumTime_{ 0.0f };
    bool isRewind_{ false };
    float rotAccumAngle_{ 0.0f };
    float downVelocity_{ 0.0f };
    double angle_{ 0.0f };

    SDL_FPoint renderTargetPos_{};

};