#pragma once
#include <cstdint>

// 기본 블록 인터페이스
class IBlock {
public:
    virtual ~IBlock() = default;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
};

// 블록 타입 enum class로 현대화
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

// 블록 상태 enum class
enum class BlockState {
    Playing,
    Effecting,
    Stationary,
    Destroying,
    DownMoving,
    PlayOut,
    Max
};

// 블록 이펙트 상태
enum class EffectState {
    None = -1,
    Sparkle,
    Compress,
    Expand,
    Destroy,
    Max
};

// 블록 링크 상태를 비트 플래그로 
enum class LinkState : uint8_t {
    None = 0,
    Left = 1 << 0,
    Top = 1 << 1,
    Right = 1 << 2,
    Bottom = 1 << 3
};