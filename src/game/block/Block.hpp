#pragma once

/**
 *
 * 설명: Game Block 오브젝트 1개를 구성한 Class
 * 
 */

#include <memory>
#include <cstdint>
#include <array>
#include <functional>
#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"

// 블록 타입 enum class
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
enum class BlockState
{
    Playing,    // 플레이어 조작 상태
    Effecting,  // 이펙트 재생 상태
    Stationary, // 고정된 상태
    Destroying, // 파괴중인 상태
    DownMoving, // 아래로 이동중인 상태
    PlayOut,    // 게임에서 제거된 상태
    Max
};


// 블록 이펙트 상태
enum class EffectState {
    None = -1,
    Sparkle,
    Compress,   // 작아지는 상태
    Expand,     // 확장 커지는 상태
    Destroy,    // 삭제중인 상태
    Max
};


// 주변 블록들의 타입에 따른 블록 상태 갱신 Bit 연산
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
    void SetEffectState(EffectState state) { effect_state_ = state; }
    void SetLevel(uint8_t level) { level_ = level; }

    void SetPosIdx(int x, int y)
    {
		index_x_ = x;
		index_y_ = y;
    }
    void SetPosIdx_X(int x)
    {
		index_x_ = x;
    }

    void SetScale(float width, float height) override;

    
    [[nodiscard]] uint8_t GetPlayerId() const { return playerID_; }
    [[nodiscard]] int GetPosIdx_X() const { return index_x_; }
    [[nodiscard]] int GetPosIdx_Y() const { return index_y_; }
    [[nodiscard]] BlockType GetBlockType() const { return block_type_; }
    [[nodiscard]] BlockState GetState() const { return state_; }
    [[nodiscard]] LinkState GetLinkState() const { return link_state_; }
    [[nodiscard]] EffectState GetEffectState() const { return effect_state_; }

    void SetRecursionCheck(bool check) { is_recursionCheck_ = check; }
    [[nodiscard]] bool IsRecursionCheck() const { return is_recursionCheck_; }

    void SetStandard(bool standard) { is_standard_ = standard; }
    [[nodiscard]] bool IsStandard() const { return is_standard_; }   

private:
    void UpdateBlockEffect(float deltaTime);     // 블록 이펙트 업데이트
    void UpdateDestroying(float deltaTime);      // 파괴 상태 업데이트
    void UpdateDestroyingExpand(float deltaTime);
    void UpdateDestroyingRotate(float deltaTime);

    void UpdateDownMoving(float deltaTime);      // 낙하 상태 업데이트
    void UpdatePlayingState(float deltaTime);    // 플레이 상태 업데이트
    void InitializeEffectPositions();            // 이펙트 위치 초기화
    void UpdateSourceRectForLinkState();
    void UpdateLinkStateForDownMoving();    
    

protected:

    SDL_FRect source_rect_;                       // 텍스처 소스 영역
    SDL_FPoint block_origin_Position_;                  // 블록 원점 위치
    std::array<SDL_FPoint, static_cast<int>(EffectState::Max)> block_effect_position_; // 이펙트 위치

    BlockType block_type_{ BlockType::Max };      // 블록 타입
    BlockState state_{ BlockState::Max };        // 블록 상태
    LinkState link_state_{ LinkState::Normal };   // 링크 상태
    EffectState effect_state_{ EffectState::None };// 이펙트 상태

    uint8_t level_{ 0 };                         // 블록 레벨
    std::shared_ptr<ImageTexture> texture_;      // 블록 텍스처

    bool is_scaled_{ false };                     // 크기 변경 여부
    bool is_recursionCheck_{ false };             // 재귀 체크 여부
    bool is_standard_{ false };                   // 표준 블록 여부
    bool is_changed_{ false };                    // 변경 여부

    int index_x_{ -1 };                           // X 인덱스
    int index_y_{ -1 };                           // Y 인덱스

    float accumulate_time_{ 0.0f };                    // 누적 시간
    float accumulate_effect_time_{ 0.0f };              // 이펙트 누적 시간
    float rotation_angle_{ 0.0f };                // 회전 각도
    float scale_velocity_{ 0.0f };                // 크기 변경 속도
    float down_velocity_{ 0.0f };                 // 낙하 속도

    uint8_t playerID_{ 0 };                      // 플레이어 ID
};
