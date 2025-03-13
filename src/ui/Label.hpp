#pragma once

#include "../game/RenderableObject.hpp"
#include "../texture/StringTexture.hpp"

#include <string>
#include <memory>
#include <SDL3/SDL.h>

enum class TextAlignment
{
    Left,
    Center,
    Right
};

class Label : public RenderableObject
{
public:
    Label();
    ~Label() override;

    Label(const Label&) = delete;
    Label& operator=(const Label&) = delete;
    Label(Label&&) noexcept = default;
    Label& operator=(Label&&) noexcept = default;

    bool Init(float x, float y, float width, float height);
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    void SetText(std::string_view text);
    void SetAlignment(TextAlignment alignment);
    void SetTextColor(const SDL_Color& color);
    void SetBackgroundColor(const SDL_Color& color);
    void SetBackgroundVisible(bool visible);

    // 여러 옵션을 한번에 설정하는 편의 메서드
    void Configure(std::string_view text, TextAlignment alignment, const SDL_Color& textColor);

    [[nodiscard]] std::string_view GetText() const noexcept { return text_; }
    [[nodiscard]] TextAlignment GetAlignment() const noexcept { return alignment_; }
    [[nodiscard]] const SDL_Color& GetTextColor() const noexcept { return text_color_; }
    [[nodiscard]] const SDL_Color& GetBackgroundColor() const noexcept { return background_color_; }
    [[nodiscard]] bool IsBackgroundVisible() const noexcept { return show_background_; }

private:
    void UpdateTextPosition();
    void UpdateTextTexture();

private:
    std::unique_ptr<StringTexture> text_texture_;
    std::string text_{};
    SDL_Color text_color_{ 255, 255, 255, 255 };
    SDL_Color background_color_{ 0, 0, 0, 128 };
    TextAlignment alignment_{ TextAlignment::Left };
    bool show_background_{ false };
    float text_x_offset_{ 0.0f };
    float text_y_offset_{ 0.0f };
};