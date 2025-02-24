#pragma once
/*
 *
 * 설명: 방해블록 개수에 따른 UI 표시 뷰
 *
 */

#include "../RenderableObject.hpp"
#include <array>
#include <memory>

class ImageTexture;

// 방해 블록 타입 정의
enum class InterruptBlockType : uint8_t 
{
    Small = 0,  // 1개
    Middle = 1,  // 6개
    Brick = 2,  // 30개
    Star = 3,  // 180개
    Moon = 4,  // 360개
    Crown = 5,  // 720개
    Max = 6
};

// 방해 블록 표시 상태
enum class InterruptViewState : uint8_t 
{
    Normal,
    Updating
};

// 방해 블록 정보 구조체
struct InterruptBlock 
{
    SDL_FRect rect{};
    float height{ 0.0f };
};

class InterruptBlockView final : public RenderableObject
{

private:
    static constexpr size_t MAX_BLOCKS = static_cast<size_t>(InterruptBlockType::Max);
    static constexpr float ANIMATION_SPEED = 200.0f;

    struct BlockValue 
    {
        InterruptBlockType type;
        int16_t value;
    };

    static constexpr std::array<BlockValue, MAX_BLOCKS> BLOCK_VALUES =
    { {
        {InterruptBlockType::Crown,  720},
        {InterruptBlockType::Moon,   360},
        {InterruptBlockType::Star,   180},
        {InterruptBlockType::Brick,   30},
        {InterruptBlockType::Middle,   6},
        {InterruptBlockType::Small,    1}
    } };

public:

    InterruptBlockView() = default;
    ~InterruptBlockView() override = default;

    InterruptBlockView(const InterruptBlockView&) = delete;
    InterruptBlockView& operator=(const InterruptBlockView&) = delete;
    InterruptBlockView(InterruptBlockView&&) noexcept = delete;
    InterruptBlockView& operator=(InterruptBlockView&&) noexcept = delete;

    bool Initialize();
    void UpdateInterruptBlock(int16_t count);
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

private:
    void UpdateBlockRect(size_t index, InterruptBlockType type);
    SDL_Point GetBlockTexturePosition(InterruptBlockType type) const;

    std::array<InterruptBlock, MAX_BLOCKS> blocks_{};
    std::shared_ptr<ImageTexture> blockTexture_;
    uint8_t viewCount_{ 0 };
    InterruptViewState state_{ InterruptViewState::Normal };
};
