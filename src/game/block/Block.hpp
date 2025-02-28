#pragma once

/**
 *
 * ����: Game Block ������Ʈ 1���� ������ Class
 * 
 */

#include <memory>
#include <cstdint>
#include <array>
#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"

// ��� Ÿ�� enum class
enum class BlockType {
    Empty,
    Red,
    Green,
    Blue,
    Yellow,
    Purple,
    Max,
    Ice
};

// ��� ���� enum class
enum class BlockState
{
    Playing,    // �÷��̾� ���� ����
    Effecting,  // ����Ʈ ��� ����
    Stationary, // ������ ����
    Destroying, // �ı����� ����
    DownMoving, // �Ʒ��� �̵����� ����
    PlayOut,    // ���ӿ��� ���ŵ� ����
    Max
};


// ��� ����Ʈ ����
enum class EffectState {
    None = -1,
    Sparkle,
    Compress,   // �۾����� ����
    Expand,     // Ȯ�� Ŀ���� ����
    Destroy,    // �������� ����
    Max
};


// �ֺ� ��ϵ��� Ÿ�Կ� ���� ��� ���� ���� Bit ����
enum class LinkState 
{
    Normal = 0,
    Left = (1 << 0),
    Top = (1 << 1),
    Right = (1 << 2),
    Bottom = (1 << 3),
    RightTop = Right | Top,
    RightBottom = Right | Bottom,
    TopBottom = Top | Bottom,
    LeftTop = Left | Top,
    LeftBottom = Left | Bottom,
    LeftRight = Left | Right,
    RightTopBottom = Right | Top | Bottom,
    LeftTopBottom = Left | Top | Bottom,
    LeftRightTop = Left | Right | Top,
    LeftRightBottom = Left | Right | Bottom,
    LeftRightTopBottom = Left | Right | Bottom | Top,
    Max
};

class Block : public RenderableObject 
{
public:
    Block();
    virtual ~Block() = default;

    Block(const Block& other);    
    Block(Block&& other) noexcept = default;
    Block& operator=(Block&& other) noexcept = default;
    Block& operator=(const Block& other);
    [[nodiscard]] bool operator<(const Block& rhs) const;

    
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    [[nodiscard]] std::shared_ptr<Block> Clone() const;

    virtual void SetPlayerID(uint8_t id) { playerID_ = id; }
    virtual void SetState(BlockState state);    
    void SetBlockType(BlockType type);    
    void SetLinkState(LinkState state = LinkState::Normal);
    void SetBlockTex(const std::shared_ptr<ImageTexture>& tex) { texture_ = tex; }
    void SetEffectState(EffectState state) { effectState_ = state; }
    void SetLevel(uint8_t level) { level_ = level; }

    void SetPosIdx(int x, int y)
    {
		indexX_ = x;
		indexY_ = y;
    }
    void SetPosIdx_X(int x)
    {
		indexX_ = x;
    }

    void SetScale(float width, float height) override;

    [[nodiscard]] int GetPosIdx_X() const { return indexX_; }
    [[nodiscard]] int GetPosIdx_Y() const { return indexY_; }
    [[nodiscard]] BlockType GetBlockType() const { return blockType_; }
    [[nodiscard]] BlockState GetState() const { return state_; }
    [[nodiscard]] LinkState GetLinkState() const { return linkState_; }
    [[nodiscard]] EffectState GetEffectState() const { return effectState_; }

    void SetRecursionCheck(bool check) { isRecursionCheck_ = check; }
    [[nodiscard]] bool IsRecursionCheck() const { return isRecursionCheck_; }

    void SetStandard(bool standard) { isStandard_ = standard; }
    [[nodiscard]] bool IsStandard() const { return isStandard_; }   

private:
    void UpdateBlockEffect(float deltaTime);     // ��� ����Ʈ ������Ʈ
    void UpdateDestroying(float deltaTime);      // �ı� ���� ������Ʈ
    void UpdateDestroyingExpand(float deltaTime);
    void UpdateDestroyingRotate(float deltaTime);

    void UpdateDownMoving(float deltaTime);      // ���� ���� ������Ʈ
    void UpdatePlayingState(float deltaTime);    // �÷��� ���� ������Ʈ
    void InitializeEffectPositions();            // ����Ʈ ��ġ �ʱ�ȭ
    void UpdateSourceRectForLinkState();
    void UpdateLinkStateForDownMoving();    
    

protected:

    SDL_FRect sourceRect_;                       // �ؽ�ó �ҽ� ����
    SDL_FPoint blockOriginPos_;                  // ��� ���� ��ġ
    std::array<SDL_FPoint, static_cast<int>(EffectState::Max)> blockEffectPos_; // ����Ʈ ��ġ

    BlockType blockType_{ BlockType::Max };      // ��� Ÿ��
    BlockState state_{ BlockState::Max };        // ��� ����
    LinkState linkState_{ LinkState::Normal };   // ��ũ ����
    EffectState effectState_{ EffectState::None };// ����Ʈ ����

    uint8_t level_{ 0 };                         // ��� ����
    std::shared_ptr<ImageTexture> texture_;      // ��� �ؽ�ó

    bool isScaled_{ false };                     // ũ�� ���� ����
    bool isRecursionCheck_{ false };             // ��� üũ ����
    bool isStandard_{ false };                   // ǥ�� ��� ����
    bool isChanged_{ false };                    // ���� ����

    int indexX_{ -1 };                           // X �ε���
    int indexY_{ -1 };                           // Y �ε���

    float accumTime_{ 0.0f };                    // ���� �ð�
    float accumEffectTime_{ 0.0f };              // ����Ʈ ���� �ð�
    float rotationAngle_{ 0.0f };                // ȸ�� ����
    float scaleVelocity_{ 0.0f };                // ũ�� ���� �ӵ�
    float downVelocity_{ 0.0f };                 // ���� �ӵ�

    uint8_t playerID_{ 0 };                      // �÷��̾� ID
};