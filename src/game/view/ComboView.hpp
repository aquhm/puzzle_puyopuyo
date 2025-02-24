#pragma once
/*
 *
 * 설명: 블록 삭제 콤보 UI 표시
 *
 */

#include "../RenderableObject.hpp"
#include <array>
#include <memory>

class ImageTexture;

enum class ComboViewState : uint8_t {
    Normal,
    Updating
};

class ComboView final : public RenderableObject 
{
private:
    static constexpr size_t MAX_DIGITS = 3;
    static constexpr float DISPLAY_DURATION = 1.0f;
    static constexpr float DIGIT_WIDTH = 30.0f;
    static constexpr float DIGIT_HEIGHT = 58.0f;
    static constexpr float COMBO_TEXT_Y = 64.0f;
    static constexpr float COMBO_TEXT_WIDTH = 86.0f;
    static constexpr float COMBO_TEXT_HEIGHT = 36.0f;

public:

    ComboView() = default;
    ~ComboView() override = default;

    ComboView(const ComboView&) = delete;
    ComboView& operator=(const ComboView&) = delete;
    ComboView(ComboView&&) noexcept = delete;
    ComboView& operator=(ComboView&&) noexcept = delete;

    bool Initialize();
    void UpdateComboCount(float xPos, float yPos, int16_t count);
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

private:
    void InitializeRects();
    void UpdateDigitRect(size_t index, int digit);
    void UpdateComboTextRect(size_t index);

    uint8_t viewCount_{ 0 };
    float accumTime_{ 0.0f };
    std::array<SDL_FRect, MAX_DIGITS> comboRects_{};
    std::shared_ptr<ImageTexture> comboImage_;
    ComboViewState state_{ ComboViewState::Normal };
};