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
    LocalPlayer();
    ~LocalPlayer() override;

    bool Initialize(const std::span<const uint8_t>& blockType1,
        const std::span<const uint8_t>& blockType2,
        uint8_t playerIdx,
        uint16_t characterIdx,
        const std::shared_ptr<GameBackground>& background) override;

    void Release() override;
    bool Restart(const std::span<const uint8_t>& blockType1 = {}, const std::span<const uint8_t>& blockType2 = {}) override;
    void CreateNextBlock() override;
    void PlayNextBlock() override;
    bool CheckGameBlockState() override;

    void MoveBlock(uint8_t moveType, float position) override;
    void RotateBlock(uint8_t rotateType, bool horizontalMoving) override;
    void UpdateBlockPosition(float pos1, float pos2) override;
    void UpdateFallingBlock(uint8_t fallingIdx, bool falling) override;
    void ChangeBlockState(uint8_t state) override;
    bool PushBlockInGame(GameGroupBlock* groupBlock);

    void CreateBullet(Block* block, bool isAttacking) override;
    void AttackInterruptBlock(float x, float y, uint8_t type) override;
    void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) override;
    void CollectRemoveIceBlocks() override;
    

    // 로컬 플레이어 특화 메서드
    void UpdateGameLogic(float deltaTime);
    void UpdateIceBlockPhase(float deltaTime);
    void UpdateShatteringPhase(float deltaTime);
    void HandlePhaseTransition(GamePhase newPhase);
    void UpdateTargetPosIdx();

    const std::deque<std::shared_ptr<GroupBlock>>& GetNextBlock() { return next_blocks_; }

private:
    // 초기화 관련 메서드
    void InitializeNextBlocks();

    // 블록 매칭 관련 메서드
    bool FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups);
    short RecursionCheckBlock(short x, short y, Constants::Direction direction, std::vector<Block*>& matchedBlocks);

    // 점수 계산 및 상태 관리 메서드
    void CalculateScore();
    void UpdateComboState();
    void ResetComboState();
    void UpdateInterruptBlockState();

    // 방해 블록 관리 
    void GenerateIceBlocks();
    void GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture, int x, int y, uint8_t playerID);
    bool IsGameOver() const;
    bool ProcessGameOver();

private:
    // 게임 데이터
    std::deque<std::shared_ptr<GroupBlock>> next_blocks_;
    std::set<std::shared_ptr<IceBlock>> ice_blocks_;
    std::vector<BulletEffect*> bullets_to_delete_;
    std::vector<std::vector<Block*>> matched_blocks_;

    // 상태 변수
    uint64_t lastInputTime_{ 0 };
};