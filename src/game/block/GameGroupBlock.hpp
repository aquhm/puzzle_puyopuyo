#pragma once

#include <memory>
#include <list>
#include "GroupBlock.hpp"

// ���� ��� �ӵ� ���� ���
constexpr float GAME_BLOCK_DOWN_VELOCITY = 0.5f;        // ��� �ϰ� �ӵ�
constexpr float GAME_BLOCK_ROTATE_VELOCITY = 500.0f;    // ��� ȸ�� �ӵ�
constexpr float GAME_BLOCK_HORIZONTAL_VELOCITY = BlockConstants::SIZE / 90.0f; // ���� �̵� �ӵ�

// ȸ�� ����
enum class RotateState {
    Default,
    Right,
    Top,
    Left
};

// ��� �ε���
enum BlockIndex {
    StandardIdx = 0,
    SatelliteIdx = 1
};

class GameGroupBlock : public GroupBlock {
public:
    GameGroupBlock();
    ~GameGroupBlock() override = default;

    // ����/�̵� ������ ����
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
    RotateState rotateState_{ RotateState::Default };   // ���� ȸ�� ����

    bool isFalling_{ false };             // ���� ������ ����
    bool isRotating_{ false };            // ȸ�� ������ ����
    bool isHorizontalMoving_{ false };    // ���� �̵� ������ ����
    bool checkingCollision_{ false };     // �浹 üũ ������ ����
    bool canMove_{ true };                // �̵� ���� ����

    int fallingIdx_{ -1 };                // ���� ���� ��� �ε���
    int blockIndexX_{ 0 };                // ��� X �ε���
    uint8_t playerID_{ 0 };               // �÷��̾� ID
    uint32_t updateTime_{ 0 };            // ������Ʈ �ð�

    float velocity_{ 0.0f };              // ���� �ӵ�
    float addVelocity_{ 1.0f };           // �߰� �ӵ�
    float rotateVelocity_{ 0.0f };        // ȸ�� �ӵ�
    float horizontalVelocity_{ 0.0f };    // ���� �̵� �ӵ�

    SDL_Rect intersectResultRect_{};       // �浹 ��� ����� rect
    std::list<Block*>* gameBlockList_{ nullptr }; // ���� ��� ����Ʈ

    void HandleRotation(float deltaTime);  // ȸ�� ó��
    void HandleFalling(float deltaTime);   // ���� ó��
    bool CheckCollision(const SDL_Rect& rect); // �浹 üũ
    void UpdateBlockPositions();           // ��� ��ġ ������Ʈ
};