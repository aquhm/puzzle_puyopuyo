#pragma once
/**
 *
 * 설명: Block 2개로 이루어진 그룹 블록을 컨트롤 한느 Class
 *
 */
#include <memory>
#include <list>
#include "GroupBlock.hpp"
#include "../../core/common/constants/Constants.hpp"


// 회전 상태
enum class RotateState 
{
    Default,
    Right,
    Top,
    Left
};

// 블록 인덱스
enum BlockIndex 
{
    Standard = 0,
    Satellite = 1
};

class GameGroupBlock : public GroupBlock 
{
public:
    GameGroupBlock();
    ~GameGroupBlock() override = default;

    
    GameGroupBlock(const GameGroupBlock&) = delete;
    GameGroupBlock& operator=(const GameGroupBlock&) = delete;
    GameGroupBlock(GameGroupBlock&&) = delete;
    GameGroupBlock& operator=(GameGroupBlock&&) = delete;


    void Update(float deltaTime) override;
    void SetPosXY(float x, float y) override;
    void SetPosX(float x) override;
    void SetPosY(float y) override;
    void MoveLeft(bool collisionCheck = true);
    void MoveRight(bool collisionCheck = true);
    bool MoveDown(bool collisionCheck = true);
    void Rotate();

    void ForceVelocityY(float velocity);
    void ForceAddVelocityY(float velocity, bool send = true);    
    void SetAddVelocityY(float velocity) { addVelocity_ = velocity; }
    [[nodiscard]] float GetAddForceVelocityY() const { return addVelocity_; }

    void SetEnableRotState(RotateState state = RotateState::Default, bool horizontalMoving = false, bool enable = true, bool send = true);
    [[nodiscard]] RotateState GetRotateState() const { return rotateState_; }
    [[nodiscard]] float GetPosXOfIdx(int idx) const;
    [[nodiscard]] int CalculateIdxY(float y) const;

    void SetGroupBlock(GroupBlock* block);
    void SetGameBlocks(std::list<std::shared_ptr<Block>>* gameBlocks) { gameBlockList_ = gameBlocks; }
    void SetEffectState(EffectState state);
    void ResetBlock();
    void SetPlayerID(uint8_t id);
    void UpdateFallingBlock(uint8_t fallingIdx, bool falling);

protected:
    void GetCollisionRect(Block* block, SDL_Rect* rect, Constants::Direction dir);

private:
    void HandleHorizontalCollision();
    void ProcessHorizontalCollisionResult(bool collision1, bool collision2);
    void HandleSingleBlockFalling();

    void HandleRotation(float deltaTime);
    void HandleHorizontalMovement(float rotVelocity);
    void HandleEffectingState();
    void HandleDefaultTopRotation();

    //void UpdateBlockPositions();
    void UpdateBlockIndices();
    void ResetVelocities();
    void ProcessBlockPlacement();
    //bool CheckBlockCollision(const SDL_FRect& rect);

private:

    RotateState rotateState_{ RotateState::Default };
    bool isFalling_{ false };
    bool isRotating_{ false };
    bool isHorizontalMoving_{ false };
    bool checkingCollision_{ false };
    bool canMove_{ true };

    int fallingIdx_{ -1 };
    int blockIndexX_{ 0 };
    uint8_t playerID_{ 0 };
    uint64_t updateTime_{ 0 };

    float velocity_{ 0.0f };
    float addVelocity_{ 1.0f };
    float rotateVelocity_{ 0.0f };
    float horizontalVelocity_{ 0.0f };

    SDL_FRect intersectResultRect_{};
    std::list<std::shared_ptr<Block>>* gameBlockList_;
};