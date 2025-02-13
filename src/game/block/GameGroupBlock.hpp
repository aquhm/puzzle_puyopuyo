#pragma once

#include <memory>
#include <list>
#include "GroupBlock.hpp"

// 게임 블록 속도 관련 상수
constexpr float GAME_BLOCK_DOWN_VELOCITY = 0.5f;        // 블록 하강 속도
constexpr float GAME_BLOCK_ROTATE_VELOCITY = 500.0f;    // 블록 회전 속도
constexpr float GAME_BLOCK_HORIZONTAL_VELOCITY = BlockConstants::SIZE / 90.0f; // 수평 이동 속도

// 회전 상태
enum class RotateState {
    Default,
    Right,
    Top,
    Left
};

// 블록 인덱스
enum BlockIndex {
    StandardIdx = 0,
    SatelliteIdx = 1
};

class GameGroupBlock : public GroupBlock {
public:
    GameGroupBlock();
    ~GameGroupBlock() override = default;

    // 복사/이동 연산자 삭제
    GameGroupBlock(const GameGroupBlock&) = delete;
    GameGroupBlock& operator=(const GameGroupBlock&) = delete;
    GameGroupBlock(GameGroupBlock&&) noexcept = delete;
    GameGroupBlock& operator=(GameGroupBlock&&) noexcept = delete;

    // Core functionality
    void Update(float deltaTime) override;
    void SetPosX(int x) override;
    void SetPosY(int y) override;
    void SetPosXY(int x, int y) override;

    // Movement control
    void MoveLeft(bool collisionCheck = true);
    void MoveRight(bool collisionCheck = true);
    bool MoveDown(bool collisionCheck = true);
    void Rotate();

    // Force movement
    void ForceVelocityY(float velocity);
    void ForceAddVelocityY(float velocity, bool send = true);
    [[nodiscard]] float GetAddForceVelocityY() const { return addVelocity_; }
    void SetAddVelocityY(float velocity) { addVelocity_ = velocity; }

    // Rotation control
    void SetEnableRotState(RotateState state = RotateState::Default, bool bHorizontalMoving = false, bool enable = true, bool send = true);
    [[nodiscard]] RotateState GetRotateState() const { return rotateState_; }

    // Position and index management
    [[nodiscard]] float GetPosXOfIdx(int idx) const;
    [[nodiscard]] int CalculateIdxY(float y) const;

    // Block management
    void SetGroupBlock(GroupBlock* pblock);
    void SetGameBlocks(std::list<Block*>* pGameBlocks);
    void SetEffectState(EffectState state);
    void ResetBlock();

    // Player ID management
    void SetPlayerID(uint8_t id);

    // Network related
    void UpdateFallingBlock(uint8_t fallingIdx, bool falling);

protected:
    void GetCollisionRect(Block* block, SDL_Rect* rect, Direction dir);

private:
    RotateState rotateState_{ RotateState::Default };   // 현재 회전 상태

    bool isFalling_{ false };             // 낙하 중인지 여부
    bool isRotating_{ false };            // 회전 중인지 여부
    bool isHorizontalMoving_{ false };    // 수평 이동 중인지 여부
    bool checkingCollision_{ false };     // 충돌 체크 중인지 여부
    bool canMove_{ true };                // 이동 가능 여부

    int fallingIdx_{ -1 };                // 낙하 중인 블록 인덱스
    int blockIndexX_{ 0 };                // 블록 X 인덱스
    uint8_t playerID_{ 0 };               // 플레이어 ID
    uint32_t updateTime_{ 0 };            // 업데이트 시간

    float velocity_{ 0.0f };              // 현재 속도
    float addVelocity_{ 1.0f };           // 추가 속도
    float rotateVelocity_{ 0.0f };        // 회전 속도
    float horizontalVelocity_{ 0.0f };    // 수평 이동 속도

    SDL_Rect intersectResultRect_{};       // 충돌 결과 저장용 rect
    std::list<Block*>* gameBlockList_{ nullptr }; // 게임 블록 리스트

    void HandleRotation(float deltaTime);  // 회전 처리
    void HandleFalling(float deltaTime);   // 낙하 처리
    bool CheckCollision(const SDL_Rect& rect); // 충돌 체크
    void UpdateBlockPositions();           // 블록 위치 업데이트
};