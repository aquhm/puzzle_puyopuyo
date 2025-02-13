#pragma once

#include <memory>
#include <array>
#include "../RenderableObject.hpp"
#include "Block.hpp"

// ���� ��� ǥ�� ��ġ ���
constexpr int NEXT_BLOCK_POS_X = 16;
constexpr int NEXT_BLOCK_POS_Y = 10;
constexpr int NEXT_BLOCK_POS_SMALL_X = 42;
constexpr int NEXT_BLOCK_POS_SMALL_Y = 75;

constexpr int NEXT_PLAYER_BLOCK_POS_X = 111;
constexpr int NEXT_PLAYER_BLOCK_POS_Y = 10;
constexpr int NEXT_PLAYER_BLOCK_POS_SMALL_X = 94;
constexpr int NEXT_PLAYER_BLOCK_POS_SMALL_Y = 75;

constexpr int NEXT_BLOCK_SMALL_SIZE = 21;

// �׷� ��� ���� ���
constexpr size_t GROUP_BLOCK_COUNT = 2;


// �׷� ��� Ÿ��
enum class GroupBlockType {
    Default,
    Double,
    Triple,
    Quadruple,
    Half
};

class GroupBlock : public RenderableObject {
public:
    GroupBlock();
    virtual ~GroupBlock() = default;

    // ����/�̵� ������ ����
    GroupBlock(const GroupBlock&) = delete;
    GroupBlock& operator=(const GroupBlock&) = delete;
    GroupBlock(GroupBlock&&) noexcept = delete;
    GroupBlock& operator=(GroupBlock&&) noexcept = delete;

    // �ھ� ���
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void Release() override;

    // ��� ����/�ʱ�ȭ
    bool Create();
    bool Create(BlockType type1, BlockType type2);

    // ���� ����/��ȸ
    void SetState(BlockState state);
    [[nodiscard]] BlockState GetState() const { return state_; }

    // ��ġ ����
    virtual void SetPosXY(int x, int y);
    virtual void SetPosX(int x);
    virtual void SetPosY(int y);
    virtual void SetScale(int width, int height);

    // ��� Ÿ�� ����/��ȸ
    void SetType(GroupBlockType type) { groupBlockType_ = type; }
    [[nodiscard]] GroupBlockType GetType() const { return groupBlockType_; }

    // ��� ������
    [[nodiscard]] Block* GetBlock(int index);
    [[nodiscard]] const std::array<std::unique_ptr<Block>, GROUP_BLOCK_COUNT>& GetBlocks() const { return blocks_; }

protected:
    std::array<std::unique_ptr<Block>, GROUP_BLOCK_COUNT> blocks_; 
    GroupBlockType groupBlockType_;                                
    BlockState state_;                                             

    void UpdateDestRect();                       

private:
    bool ValidateBlockIndex(int index) const;    
    void InitializeBlocks();                     
};