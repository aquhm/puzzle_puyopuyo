#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <cstdint>
#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"

// Block 관련 상수 정의
namespace BlockConstants {
    constexpr float SIZE = 31.0f;                // 블록 기본 크기
    constexpr float GRAVITY = 9.8f;              // 중력 가속도
    constexpr float CHANGE_TIME = 0.2f;          // 블록 상태 변경 시간

    constexpr float EFFECT_COMPRESS_TIME = 0.1f; // 압축 이펙트 시간
    constexpr float EFFECT_EXPAND_TIME = 0.15f;  // 확장 이펙트 시간 
    constexpr float EFFECT_DESTROY_TIME = 0.5f;  // 파괴 이펙트 시간

    constexpr int DESTROY_DELTA_SIZE = 21;       // 파괴시 크기 변화량
    constexpr float DESTROY_POS_VELOCITY = (DESTROY_DELTA_SIZE * 0.5f) / EFFECT_DESTROY_TIME;

    constexpr float DESTROY_EXPAND_TIME = 0.5f;  // 파괴 확장 시간
    constexpr int DESTROY_EXPAND_DELTA_SIZE = 10;// 파괴 확장 크기
    constexpr float DESTROY_EXPAND_POS_VELOCITY = (DESTROY_EXPAND_DELTA_SIZE * 0.5f) / DESTROY_EXPAND_TIME;

    constexpr int SHATTERING_DOWN_SPEED = 50;    // 파괴 후 낙하 속도
}

// 블록 타입 enum class
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

// 블록 상태 enum class
enum class BlockState {
    Playing,    // 플레이어 조작 상태
    Effecting,  // 이펙트 재생 상태
    Stationary, // 고정된 상태
    Destroying, // 파괴중인 상태
    DownMoving, // 아래로 이동중인 상태
    PlayOut,    // 게임에서 제거된 상태
    Max
};

// 이펙트 상태 enum class
enum class EffectState {
    None = -1,
    Sparkle,
    Compress,
    Expand,
    Destroy,
    Max
};

// 기존 링크 상태 유지 (게임 로직상 필요)
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

    // 복사/이동 연산자 정의
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
    SDL_Rect sourceRect_;                        // 텍스처 소스 영역
    SDL_Point blockOriginPos_;                   // 블록 원점 위치
    SDL_Point blockEffectPos_[static_cast<int>(EffectState::Max)]; // 이펙트 위치

    BlockType blockType_{ BlockType::Max };      // 블록 타입
    BlockState state_{ BlockState::Max };        // 블록 상태
    LinkState linkState_{ LinkState::Normal };   // 링크 상태
    EffectState effectState_{ EffectState::None };// 이펙트 상태

    uint8_t level_{ 0 };                         // 블록 레벨
    std::shared_ptr<ImageTexture> texture_;      // 블록 텍스처

    bool isScaled_{ false };                     // 크기 변경 여부
    bool isRecursionCheck_{ false };             // 재귀 체크 여부
    bool isStandard_{ false };                   // 표준 블록 여부
    bool isChanged_{ false };                    // 변경 여부

    int indexX_{ -1 };                           // X 인덱스
    int indexY_{ -1 };                           // Y 인덱스

    float accumTime_{ 0.0f };                    // 누적 시간
    float accumEffectTime_{ 0.0f };              // 이펙트 누적 시간
    float rotationAngle_{ 0.0f };                // 회전 각도
    float scaleVelocity_{ 0.0f };                // 크기 변경 속도
    float downVelocity_{ 0.0f };                 // 낙하 속도

    uint8_t playerID_{ 0 };                      // 플레이어 ID

private:
    void UpdateBlockEffect(float deltaTime);     // 블록 이펙트 업데이트
    void UpdateDestroying(float deltaTime);      // 파괴 상태 업데이트
    void UpdateDownMoving(float deltaTime);      // 낙하 상태 업데이트
    void UpdatePlayingState(float deltaTime);    // 플레이 상태 업데이트
    void InitializeEffectPositions();            // 이펙트 위치 초기화
};