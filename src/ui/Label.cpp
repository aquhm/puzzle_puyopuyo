#include "Label.hpp"

#include "../core/GameApp.hpp"
#include "../utils/Logger.hpp"

#include <stdexcept>
#include <format>
#include <cassert>

Label::Label()
    : RenderableObject()
    , text_texture_(nullptr)
{
}

Label::~Label()
{
    text_texture_.reset();
}

bool Label::Init(float x, float y, float width, float height)
{
    try
    {
        SetPosition(x, y);
        SetScale(width, height);

        text_texture_ = std::make_unique<StringTexture>();
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize Label: %s", e.what());
        return false;
    }
}

void Label::Update(float deltaTime)
{
}

void Label::Render()
{
    if (!IsVisible() || !text_texture_)
    {
        return;
    }

    auto* renderer = GAME_APP.GetRenderer();
    if (!renderer)
    {
        return;
    }

    // 배경 렌더링
    if (show_background_)
    {
        SDL_SetRenderDrawColor(renderer,
            background_color_.r,
            background_color_.g,
            background_color_.b,
            background_color_.a);

        SDL_FRect rect = { GetX(), GetY(), GetWidth(), GetHeight() };
        SDL_RenderFillRect(renderer, &rect);
    }

    // 텍스트 렌더링
    if (text_texture_ && !text_.empty())
    {
        text_texture_->Render(GetX() + text_x_offset_, GetY() + text_y_offset_);
    }
}

void Label::Release()
{
    text_texture_.reset();
    text_.clear();
}

void Label::SetText(std::string_view text)
{
    if (text_ == text)
    {
        return; // 텍스트가 변경되지 않았으면 리턴
    }

    text_ = text;
    UpdateTextTexture();
    UpdateTextPosition();
}

void Label::SetAlignment(TextAlignment alignment)
{
    if (alignment_ == alignment)
    {
        return;
    }

    alignment_ = alignment;
    UpdateTextPosition();
}

void Label::SetTextColor(const SDL_Color& color)
{
    if (text_color_.r == color.r &&
        text_color_.g == color.g &&
        text_color_.b == color.b &&
        text_color_.a == color.a)
    {
        return;
    }

    text_color_ = color;
    UpdateTextTexture();
}

void Label::SetBackgroundColor(const SDL_Color& color)
{
    background_color_ = color;
}

void Label::SetBackgroundVisible(bool visible)
{
    show_background_ = visible;
}

void Label::Configure(std::string_view text, TextAlignment alignment, const SDL_Color& textColor)
{
    // 텍스트, 정렬, 색상을 한번에 설정
    bool textChanged = (text_ != text);
    bool colorChanged = (text_color_.r != textColor.r ||
        text_color_.g != textColor.g ||
        text_color_.b != textColor.b ||
        text_color_.a != textColor.a);

    text_ = text;
    alignment_ = alignment;
    text_color_ = textColor;

    if (textChanged || colorChanged)
    {
        UpdateTextTexture();
    }

    UpdateTextPosition();
}

void Label::UpdateTextPosition()
{
    if (!text_texture_ || text_.empty())
    {
        return;
    }

    // 수평 정렬 계산
    switch (alignment_)
    {
    case TextAlignment::Left:
        text_x_offset_ = 0.0f;
        break;
    case TextAlignment::Center:
        text_x_offset_ = (GetWidth() - text_texture_->GetWidth()) / 2.0f;
        break;
    case TextAlignment::Right:
        text_x_offset_ = GetWidth() - text_texture_->GetWidth();
        break;
    }

    // 수직 중앙 정렬 (항상 Y축 중앙에 위치)
    text_y_offset_ = (GetHeight() - text_texture_->GetHeight()) / 2.0f;

    // 음수가 되지 않도록 보정
    if (text_x_offset_ < 0.0f) text_x_offset_ = 0.0f;
    if (text_y_offset_ < 0.0f) text_y_offset_ = 0.0f;
}

void Label::UpdateTextTexture()
{
    if (!text_texture_ || text_.empty())
    {
        return;
    }

    try
    {
        text_texture_->RenderText(text_, text_color_);
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to render label text: %s", e.what());
    }
}