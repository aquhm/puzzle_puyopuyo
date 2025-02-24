#include "InterruptBlockView.hpp"
#include "../../core/manager/ResourceManager.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/GameApp.hpp"

#include <cassert>

bool InterruptBlockView::Initialize() 
{

    blockTexture_ = ImageTexture::Create("PUYO/puyo_beta.png");
    
    if (!blockTexture_) 
    {
        assert(false && "Failed to load interrupt block texture");
        return false;
    }

    for (auto& block : blocks_) 
    {
        block.rect.w = block.rect.h = Constants::Block::SIZE;
        block.height = 0;
    }

    UpdateInterruptBlock(0);
    return true;
}

SDL_Point InterruptBlockView::GetBlockTexturePosition(InterruptBlockType type) const 
{
    const int baseX = 1;
    const int baseY = 1;
    const int stride = static_cast<int>(Constants::Block::SIZE) + 1;

    switch (type) {
    case InterruptBlockType::Small:
        return { baseX + stride * 14, baseY + stride * 12 };
    case InterruptBlockType::Middle:
        return { baseX + stride * 13, baseY + stride * 12 };
    case InterruptBlockType::Brick:
        return { baseX + stride * 12, baseY + stride * 12 };
    case InterruptBlockType::Moon:
        return { baseX + stride * 12, baseY + stride * 11 };
    case InterruptBlockType::Star:
        return { baseX + stride * 11, baseY + stride * 11 };
    case InterruptBlockType::Crown:
        return { baseX + stride * 10, baseY + stride * 11 };
    default:
        return { 0, 0 };
    }
}

void InterruptBlockView::UpdateBlockRect(size_t index, InterruptBlockType type) 
{
    if (index >= MAX_BLOCKS) 
    {
        return;
    }

    const auto [x, y] = GetBlockTexturePosition(type);
    blocks_[index].rect.x = static_cast<float>(x);
    blocks_[index].rect.y = static_cast<float>(y);
}

void InterruptBlockView::UpdateInterruptBlock(int16_t count) 
{
    viewCount_ = 0;
    size_t index = 0;

    // 각 블록 타입별로 필요한 개수 계산
    for (const auto& blockValue : BLOCK_VALUES) 
    {
        while (count >= blockValue.value && index < MAX_BLOCKS) 
        {
            count -= blockValue.value;
            UpdateBlockRect(index++, blockValue.type);
        }
    }

    viewCount_ = static_cast<uint8_t>(index);

    if (viewCount_ > 0) 
    {
        state_ = InterruptViewState::Updating;

        for (size_t i = 0; i < viewCount_; ++i) 
        {
            blocks_[i].height = -static_cast<float>(Constants::Block::SIZE * (i + 1));
        }
    }
}

void InterruptBlockView::Update(float deltaTime) 
{
    if (state_ != InterruptViewState::Updating || viewCount_ == 0) 
    {
        return;
    }

    const float speed = deltaTime * ANIMATION_SPEED;
    size_t completedCount = 0;

    for (size_t i = 0; i < viewCount_; ++i) 
    {
        blocks_[i].height += speed;

        if (blocks_[i].height >= 0.0f) 
        {
            blocks_[i].height = 0.0f;
            ++completedCount;
        }
    }

    if (completedCount == viewCount_) 
    {
        state_ = InterruptViewState::Normal;
    }
}

void InterruptBlockView::Render() 
{
    if (!is_visible_ || !blockTexture_) 
    {
        return;
    }

    const float startX = GetX();

    for (size_t i = 0; i < viewCount_; ++i) 
    {
        blockTexture_->Render(startX + (Constants::Block::SIZE * i),blocks_[i].height, &blocks_[i].rect);
    }
}

void InterruptBlockView::Release() 
{
    blockTexture_.reset();
    state_ = InterruptViewState::Normal;
    viewCount_ = 0;
}