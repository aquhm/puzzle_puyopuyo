#pragma once
/**
 *
 * 설명: 로컬 플레이어 구현 (현재 사용자가 직접 조작하는 플레이어)
 *
 */
#include "BasePlayer.hpp"
#include <vector>
#include <set>
#include <deque>

class Block;
class GroupBlock;
class IceBlock;
class BulletEffect;
class ImageTexture;

class LocalPlayer : public BasePlayer 
{
public:
    LocalPlayer() = default;
    ~LocalPlayer() override;

    bool Initialize(const std::span<const uint8_t>& blockType1,
        const std::span<const uint8_t>& blockType2,
        uint8_t playerIdx,
        uint16_t characterIdx,
        const std::shared_ptr<GameBackground>& background) override;

    void Update(float deltaTime) override;
    void Release() override;
    void Reset() override;
    bool Restart(const std::span<const uint8_t>& blockType1 = {}, const std::span<const uint8_t>& blockType2 = {}) override;
    void CreateNextBlock() override;
    void PlayNextBlock() override;
    void MoveBlock(uint8_t moveType, float position) override;
    void RotateBlock(uint8_t rotateType, bool horizontalMoving) override;
    void UpdateBlockPosition(float pos1, float pos2) override;
    void UpdateFallingBlock(uint8_t fallingIdx, bool falling) override;
    void ChangeBlockState(uint8_t state) override;
    void CreateBullet(Block* block) override;
    void AttackInterruptBlock(float x, float y, uint8_t type) override;
    void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) override;
    void AddInterruptBlockCount(int16_t count, float x, float y, uint8_t type);
    bool CheckGameBlockState() override;
    
    bool PushBlockInGame(GameGroupBlock* groupBlock);    
    void UpdateGameLogic(float deltaTime);
    void UpdateIceBlockPhase(float deltaTime);
    void UpdateShatteringPhase(float deltaTime);
    void UpdateTargetPosIdx();
    const std::deque<std::shared_ptr<GroupBlock>>& GetNextBlock() { return next_blocks_; }

    // 점수 계산 및 상태 관리 메서드
    void CalculateScore();
    void UpdateInterruptBlockState();    

private:

    void InitializeNextBlocks();
    void ResetComboState() override;
    bool ProcessGameOver() override;

private:

    // 상태 변수
    uint64_t last_inputTime_{ 0 };
};