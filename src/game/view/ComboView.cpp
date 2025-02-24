#include "ComboView.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../../core/manager/ResourceManager.hpp"
#include <cassert>



bool ComboView::Initialize() 
{
    comboImage_ = ImageTexture::Create("PUYO/rensa_font.png");

    if (!comboImage_) 
    {
        assert(false && "ComboView::Initialize() - Failed to load texture");
        return false;
    }

    InitializeRects();
    return true;
}

void ComboView::InitializeRects() 
{
    for (auto& rect : comboRects_) 
    {
        rect = {
            0,              // x
            0,              // y
            DIGIT_WIDTH,    // w
            DIGIT_HEIGHT    // h
        };
    }

    comboRects_[2] = {
        0,                  // x
        COMBO_TEXT_Y,       // y
        COMBO_TEXT_WIDTH,   // w
        COMBO_TEXT_HEIGHT   // h
    };
}

void ComboView::UpdateDigitRect(size_t index, int digit) 
{
    if (index >= MAX_DIGITS)
    {
        return;
    }

    comboRects_[index].x = static_cast<float>(DIGIT_WIDTH * digit);
    comboRects_[index].y = 0.0f;
    comboRects_[index].w = DIGIT_WIDTH;
    comboRects_[index].h = DIGIT_HEIGHT;
}

void ComboView::UpdateComboTextRect(size_t index) 
{
    if (index >= MAX_DIGITS)
    {
        return;
    }

    comboRects_[index].x = 0;
    comboRects_[index].y = COMBO_TEXT_Y;
    comboRects_[index].w = COMBO_TEXT_WIDTH;
    comboRects_[index].h = COMBO_TEXT_HEIGHT;
}

void ComboView::UpdateComboCount(float xPos, float yPos, int16_t count) 
{
    const int16_t tens = count / 10;
    const int16_t ones = count % 10;

    if (count >= 10) 
    {
        viewCount_ = 3;
        UpdateDigitRect(0, tens);    // 십의 자리
        UpdateDigitRect(1, ones);    // 일의 자리
        UpdateComboTextRect(2);      // COMBO 텍스트
    }
    else 
    {
        viewCount_ = 2;
        UpdateDigitRect(0, ones);    // 일의 자리
        UpdateComboTextRect(1);      // COMBO 텍스트
    }

    SetPosition(xPos, yPos);
    accumTime_ = 0.0f;
    state_ = ComboViewState::Updating;
    is_visible_ = true;
}

void ComboView::Update(float deltaTime) 
{
    if (state_ == ComboViewState::Updating) 
    {
        accumTime_ += deltaTime;

        if (accumTime_ > DISPLAY_DURATION) 
        {
            accumTime_ = 0.0f;
            state_ = ComboViewState::Normal;
            is_visible_ = false;
        }
    }
}

void ComboView::Render() 
{
    if (!is_visible_ || !comboImage_)
    {
        return;
    }

    const float startX = GetX();
    const float startY = GetY();

    for (size_t i = 0; i < viewCount_; ++i) 
    {
        comboImage_->Render(startX + (DIGIT_WIDTH * i), startY, &comboRects_[i]);
    }
}

void ComboView::Release() 
{
    comboImage_.reset();

    state_ = ComboViewState::Normal;
    accumTime_ = 0.0f;
    is_visible_ = false;
}