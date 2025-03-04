//#pragma once
//
//#include "BasePlayer.hpp"
//
//class RemotePlayer : public BasePlayer
//{
//public:
//    RemotePlayer();
//    ~RemotePlayer() override = default;
//
//    bool Initialize(float boardX, float boardY) override;
//
//    void HandleMoveBlock(uint8_t moveType, float position);
//    void HandleRotateBlock(uint8_t rotateType, bool horizontalMoving);
//    void HandleBlockState(uint8_t state);
//    void UpdateBlockPosition(float pos1, float pos2);
//    bool PushBlockInGame(std::span<float> pos1, std::span<float> pos2);
//    void ChangeBlockState(uint8_t state);
//    void RotateBlock(uint8_t rotate_type, bool horizontal_moving);
//    void UpdateFallingBlock(uint8_t falling_idx, bool falling);
//    void MoveBlock(uint8_t move_type, float position);
//    bool Restart(const std::span<const uint8_t>& block_type1, const std::span<const uint8_t>& block_type2);
//    void InitializeNextBlocks(std::span<const uint8_t> block_type1, std::span<const uint8_t> block_type2);
//
//private:
//    float last_position_{ 0.0f };
//};


#pragma once
/**
 *
 * ����: ���� �÷��̾� ���� (��Ʈ��ũ�� ���� �ٸ� �÷��̾��� ������ ǥ��)
 *
 */
#include "BasePlayer.hpp"
#include <vector>
#include <set>
#include <deque>
#include <list>

class Block;
class GroupBlock;
class IceBlock;
class BulletEffect;
class ImageTexture;

class RemotePlayer : public BasePlayer {
public:
    // ��� �׷� ���� Ÿ�� ����
    using BlockVector = std::vector<std::shared_ptr<Block>>;

    RemotePlayer();
    ~RemotePlayer() override;

    // BasePlayer �������̽� ����
    bool Initialize(const std::span<const uint8_t>& blocktype1,
        const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx,
        uint8_t characterIdx,
        const std::shared_ptr<GameBackground>& background) override;

    bool Restart(const std::span<const uint8_t>& blockType1,
        const std::span<const uint8_t>& blockType2) override;

    void CreateNextBlock() override;
    void DestroyNextBlock() override;
    bool CheckGameBlockState() override;

    void MoveBlock(uint8_t moveType, float position) override;
    void RotateBlock(uint8_t rotateType, bool horizontalMoving) override;
    void UpdateBlockPosition(float pos1, float pos2) override;
    void UpdateFallingBlock(uint8_t fallingIdx, bool falling) override;
    void ChangeBlockState(uint8_t state) override;
    bool PushBlockInGame(const std::span<const float>& pos1, const std::span<const float>& pos2);
    void AddNewBlock(const std::span<const uint8_t, 2>& block_type);

    void AttackInterruptBlock(float x, float y, uint8_t type) override;
    void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) override;
    void CollectRemoveIceBlocks() override;

    // ���� ��� ���� �߰� �޼���
    void AddInterruptBlock(uint8_t y_row_cnt, std::span<uint8_t> x_idx);

    // ĳ���� ����
    void SetCharacterID(int16_t charID) { character_id_ = charID; }
    [[nodiscard]] int16_t GetCharacterID() const { return character_id_; }

    // ���� ������Ʈ �޼���
    void UpdateGameState(float deltaTime);

private:
    // �ʱ�ȭ �޼���
    void InitializeNextBlocks(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2);

    // ��� ��Ī ���� �޼���
    int16_t RecursionCheckBlock(int16_t x, int16_t y, int16_t direction, std::vector<Block*>& block_list);

    // ���� ��� ���� �޼���
    void CreateFullRowInterruptBlocks(std::shared_ptr<ImageTexture>& texture);
    void CreatePartialRowInterruptBlocks(uint8_t y_row_cnt, std::span<uint8_t> x_idx, std::shared_ptr<ImageTexture>& texture);
    void CreateSingleIceBlock(int x, int y, std::shared_ptr<ImageTexture>& texture);
    void CollectAdjacentIceBlocks(const std::shared_ptr<Block>& block);

    // ���� ���� ������Ʈ �޼���
    void UpdateStandingState(float deltaTime);
    void UpdatePlayingState(float deltaTime);
    void UpdateIceBlockDowningState();
    void UpdateShatteringState();
    void UpdateMatchedBlocks();
    void HandleClearedBlockGroup(std::list<BlockVector>::iterator& group_it, SDL_FPoint& pos, SDL_Point& pos_idx, std::list<SDL_Point>& x_index_list);
    void UpdateAfterBlocksCleared();
    void UpdateComboDisplay(const SDL_FPoint& pos);

    // ���� ���� �޼���
    void HandleMatchedBlocks();
    void ResetMatchState();
    void RemoveIceBlocks(std::list<SDL_Point>& x_index_list);
    void CalculateIceBlockCount();

private:
    // ĳ���� �� ���� ����
    int16_t character_id_{ 0 };
    bool has_ice_block_{ false };

    // ���� ������
    std::deque<std::shared_ptr<GroupBlock>> new_blocks_;
    std::set<std::shared_ptr<IceBlock>> ice_block_set_;
    std::vector<std::shared_ptr<BulletEffect>> del_bullet_array_;
    std::list<BlockVector> equal_block_list_;
};