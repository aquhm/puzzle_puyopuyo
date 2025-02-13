#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <cstdint>
#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"

// Block ���� ��� ����
namespace BlockConstants {
    constexpr float SIZE = 31.0f;                // ��� �⺻ ũ��
    constexpr float GRAVITY = 9.8f;              // �߷� ���ӵ�
    constexpr float CHANGE_TIME = 0.2f;          // ��� ���� ���� �ð�

    constexpr float EFFECT_COMPRESS_TIME = 0.1f; // ���� ����Ʈ �ð�
    constexpr float EFFECT_EXPAND_TIME = 0.15f;  // Ȯ�� ����Ʈ �ð� 
    constexpr float EFFECT_DESTROY_TIME = 0.5f;  // �ı� ����Ʈ �ð�

    constexpr int DESTROY_DELTA_SIZE = 21;       // �ı��� ũ�� ��ȭ��
    constexpr float DESTROY_POS_VELOCITY = (DESTROY_DELTA_SIZE * 0.5f) / EFFECT_DESTROY_TIME;

    constexpr float DESTROY_EXPAND_TIME = 0.5f;  // �ı� Ȯ�� �ð�
    constexpr int DESTROY_EXPAND_DELTA_SIZE = 10;// �ı� Ȯ�� ũ��
    constexpr float DESTROY_EXPAND_POS_VELOCITY = (DESTROY_EXPAND_DELTA_SIZE * 0.5f) / DESTROY_EXPAND_TIME;

    constexpr int SHATTERING_DOWN_SPEED = 50;    // �ı� �� ���� �ӵ�
}

// ��� Ÿ�� enum class
enum class BlockType : int {
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
enum class BlockState {
    Playing,    // �÷��̾� ���� ����
    Effecting,  // ����Ʈ ��� ����
    Stationary, // ������ ����
    Destroying, // �ı����� ����
    DownMoving, // �Ʒ��� �̵����� ����
    PlayOut,    // ���ӿ��� ���ŵ� ����
    Max
};

// ����Ʈ ���� enum class
enum class EffectState {
    None = -1,
    Sparkle,
    Compress,
    Expand,
    Destroy,
    Max
};

// ���� ��ũ ���� ���� (���� ������ �ʿ�)
enum class LinkState {
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

class Block : public RenderableObject {
public:
    Block();
    virtual ~Block() = default;

    // ����/�̵� ������ ����
    Block(const Block& other);
    Block& operator=(const Block& other);
    Block(Block&& other) noexcept = default;
    Block& operator=(Block&& other) noexcept = default;

    // Core functionality
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    // Block specific functionality
    void SetBlockType(BlockType type);
    void SetState(BlockState state);
    void SetLinkState(LinkState state = LinkState::Normal);
    void SetEffectState(EffectState state) { effectState_ = state; }
    void SetLevel(uint8_t level) { level_ = level; }

    // Position and Index handling
    void SetPosIdx(int x, int y);
    void SetPosIdx_X(int x);
    [[nodiscard]] int GetPosIdx_X() const { return indexX_; }
    [[nodiscard]] int GetPosIdx_Y() const { return indexY_; }

    // State queries
    [[nodiscard]] BlockType GetBlockType() const { return blockType_; }
    [[nodiscard]] BlockState GetState() const { return state_; }
    [[nodiscard]] LinkState GetLinkState() const { return linkState_; }
    [[nodiscard]] EffectState GetEffectState() const { return effectState_; }

    // Utility functions
    void SetRecursionCheck(bool check) { isRecursionCheck_ = check; }
    [[nodiscard]] bool IsRecursionCheck() const { return isRecursionCheck_; }

    void SetStandard(bool standard) { isStandard_ = standard; }
    [[nodiscard]] bool IsStandard() const { return isStandard_; }

    // Scale handling
    void SetScale(float width, float height);
    void ChangeScaleDuration(float scale, float time);

    // Comparison operator
    bool operator<(const Block& rhs) const;

protected:
    SDL_Rect sourceRect_;                        // �ؽ�ó �ҽ� ����
    SDL_Point blockOriginPos_;                   // ��� ���� ��ġ
    SDL_Point blockEffectPos_[static_cast<int>(EffectState::Max)]; // ����Ʈ ��ġ

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

private:
    void UpdateBlockEffect(float deltaTime);     // ��� ����Ʈ ������Ʈ
    void UpdateDestroying(float deltaTime);      // �ı� ���� ������Ʈ
    void UpdateDownMoving(float deltaTime);      // ���� ���� ������Ʈ
    void UpdatePlayingState(float deltaTime);    // �÷��� ���� ������Ʈ
    void InitializeEffectPositions();            // ����Ʈ ��ġ �ʱ�ȭ
};