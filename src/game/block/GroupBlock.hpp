#pragma once
/**
 *
 * 설명: Block 2개를 구성 관리하는 클래스
 *
 */

#include <memory>
#include <array>
#include "../RenderableObject.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "Block.hpp"

// 그룹 블록 타입
enum class GroupBlockType 
{
    Default,
    Double,
    Triple,
    Quadruple,
    Half
};

class GroupBlock : public RenderableObject 
{
public:
    GroupBlock();
    virtual ~GroupBlock() = default;

    GroupBlock(const GroupBlock&) = delete;
    GroupBlock& operator=(const GroupBlock&) = delete;
    GroupBlock(GroupBlock&&) noexcept = delete;
    GroupBlock& operator=(GroupBlock&&) noexcept = delete;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void Release() override;

    bool Create();
    bool Create(BlockType type1, BlockType type2);

    void SetState(BlockState state);
    [[nodiscard]] BlockState GetState() const { return state_; }

    virtual void SetPosXY(float x, float y);
    virtual void SetPosX(float x);
    virtual void SetPosY(float y);
    virtual void SetScale(float width, float height);
    void SetType(GroupBlockType type) { group_block_type_ = type; }

    [[nodiscard]] GroupBlockType GetType() const { return group_block_type_; }
    [[nodiscard]] Block* GetBlock(int index);
    [[nodiscard]] const std::array<std::shared_ptr<Block>, Constants::GroupBlock::COUNT>& GetBlocks() const { return blocks_; }

protected:
    void UpdateDestRect();         

private:
    bool ValidateBlockIndex(int index) const;
    void InitializeBlocks();

protected:

    std::array<std::shared_ptr<Block>, Constants::GroupBlock::COUNT> blocks_{};
    GroupBlockType group_block_type_{ GroupBlockType::Default };
    BlockState state_{ BlockState::Max };                 
};