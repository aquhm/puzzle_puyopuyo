#pragma once
/**
 *
 * ����: ���� �÷��̾� ���� (���� ����ڰ� ���� �����ϴ� �÷��̾�)
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
    

    // ���� �÷��̾� Ưȭ �޼���
    void UpdateGameLogic(float deltaTime);
    void UpdateIceBlockPhase(float deltaTime);
    void UpdateShatteringPhase(float deltaTime);
    void HandlePhaseTransition(GamePhase newPhase);
    void UpdateTargetPosIdx();

    const std::deque<std::shared_ptr<GroupBlock>>& GetNextBlock() { return next_blocks_; }

private:
    // �ʱ�ȭ ���� �޼���
    void InitializeNextBlocks();

    // ��� ��Ī ���� �޼���
    bool FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups);
    short RecursionCheckBlock(short x, short y, Constants::Direction direction, std::vector<Block*>& matchedBlocks);

    // ���� ��� �� ���� ���� �޼���
    void CalculateScore();
    void UpdateComboState();
    void ResetComboState();
    void UpdateInterruptBlockState();

    // ���� ��� ���� 
    void GenerateIceBlocks();
    void GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture, int x, int y, uint8_t playerID);
    bool IsGameOver() const;
    bool ProcessGameOver();

private:
    // ���� ������
    std::deque<std::shared_ptr<GroupBlock>> next_blocks_;
    std::set<std::shared_ptr<IceBlock>> ice_blocks_;
    std::vector<BulletEffect*> bullets_to_delete_;
    std::vector<std::vector<Block*>> matched_blocks_;

    // ���� ����
    uint64_t lastInputTime_{ 0 };
};