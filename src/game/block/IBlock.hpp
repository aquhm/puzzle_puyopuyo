#pragma once
#include <cstdint>

// �⺻ ��� �������̽�
class IBlock {
public:
    virtual ~IBlock() = default;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
};

// ��� Ÿ�� enum class�� ����ȭ
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
enum class BlockState {
    Playing,
    Effecting,
    Stationary,
    Destroying,
    DownMoving,
    PlayOut,
    Max
};

// ��� ����Ʈ ����
enum class EffectState {
    None = -1,
    Sparkle,
    Compress,
    Expand,
    Destroy,
    Max
};

// ��� ��ũ ���¸� ��Ʈ �÷��׷� 
enum class LinkState : uint8_t {
    None = 0,
    Left = 1 << 0,
    Top = 1 << 1,
    Right = 1 << 2,
    Bottom = 1 << 3
};