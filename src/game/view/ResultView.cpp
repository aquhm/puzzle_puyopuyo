#include "ResultView.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../../core/GameUtils.hpp"
#include "../../core/manager/ResourceManager.hpp"
#include <cassert>

bool ResultView::Initialize() 
{
    resultTexture_ = ImageTexture::Create("PUYO/result.png");
    if (!resultTexture_) 
    {
        assert(false && "Failed to load result texture");
        return false;
    }

    resultRect_ = {
        0,              // x
        0,              // y
        TEXTURE_WIDTH,  // width
        TEXTURE_HEIGHT  // height
    };

    SetScale(TEXTURE_WIDTH, TEXTURE_HEIGHT);
    
    is_visible_ = false;
    return true;
}

void ResultView::UpdateResult(float xPos, float yPos, bool isWin) 
{
    resultRect_.y = static_cast<float>(isWin ? WIN_TEXTURE_Y : LOSE_TEXTURE_Y);

    originalPosY_ = yPos;
    SetPosition(xPos, yPos);

    rotationAngle_ = 0.0f;
    is_visible_ = true;
}

void ResultView::UpdateVerticalPosition() 
{
    const float oscillation = OSCILLATION_AMPLITUDE * std::sin(GameUtils::ToRadians(rotationAngle_));
    SetY(originalPosY_ + oscillation);
}

void ResultView::Update(float deltaTime)
{
    if (!is_visible_) 
    {
        return;
    }

    rotationAngle_ += deltaTime * ROTATION_SPEED;

    if (rotationAngle_ >= FULL_ROTATION) 
    {
        rotationAngle_ -= FULL_ROTATION;
    }

    UpdateVerticalPosition();
}

void ResultView::Render() 
{
    if (!is_visible_ || !resultTexture_)
    {
        return;
    }

    resultTexture_->RenderScaled(&resultRect_, &destination_rect_);
}

void ResultView::Release() 
{
    resultTexture_.reset();
    is_visible_ = false;
    rotationAngle_ = 0.0f;
}