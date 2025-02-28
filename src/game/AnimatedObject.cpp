#include "AnimatedObject.hpp"
#include <stdexcept>

void AnimatedObject::Update(float deltaTime) 
{
    if (!animation_info_.is_playing) 
    {
        return;
    }

    animation_info_.accumulated_time += deltaTime;

    if (animation_info_.accumulated_time >= animation_info_.frame_time) 
    {
        animation_info_.accumulated_time = 0.0f;

        if (!animation_info_.is_reverse) 
        {
            animation_info_.current_frame++;
            if (animation_info_.current_frame >= animation_info_.frame_count) 
            {
                if (animation_info_.is_looping) 
                {
                    animation_info_.current_frame = 0;
                }
                else 
                {
                    animation_info_.current_frame = animation_info_.frame_count - 1;
                    Stop();
                }
            }
        }
        else 
        {
            animation_info_.current_frame--;
            
            if (animation_info_.current_frame < 0) 
            {
                if (animation_info_.is_looping) 
                {
                    animation_info_.current_frame = animation_info_.frame_count - 1;
                }
                else 
                {
                    animation_info_.current_frame = 0;
                    Stop();
                }
            }
        }

        UpdateFrameRect();
    }
}

void AnimatedObject::Render() 
{
    if (!is_visible_ || !texture_) 
    {
        return;
    }

    texture_->Render(destination_rect_.x, destination_rect_.y, &sprite_sheet_info_.view_rect);    
}

void AnimatedObject::Release() 
{
    texture_.reset();
}

void AnimatedObject::Play() 
{
    if (!texture_) 
    {
        return;
    }

    animation_info_.is_playing = true;
    animation_info_.accumulated_time = 0.0f;

    Reset();
}

void AnimatedObject::Stop() 
{
    animation_info_.is_playing = false;
    animation_info_.accumulated_time = 0.0f;
}

void AnimatedObject::Pause() 
{
    animation_info_.is_playing = false;
}

void AnimatedObject::Reset() 
{
    animation_info_.current_frame = animation_info_.is_reverse ?
        animation_info_.frame_count - 1 : 0;

    animation_info_.accumulated_time = 0.0f;
    UpdateFrameRect();
}

void AnimatedObject::SetReverse(bool reverse) 
{
    if (animation_info_.is_reverse != reverse) 
    {
        animation_info_.is_reverse = reverse;
        Reset();
    }
}

void AnimatedObject::SetTextureInfo(std::shared_ptr<ImageTexture> texture,
    const SDL_FRect& view_rect,
    const SDL_FRect& source_rect,
    const SDL_FPoint& spacing) 
{
    if (!texture) 
    {
        throw std::invalid_argument("Texture cannot be null");
    }

    texture_ = texture;
    sprite_sheet_info_.view_rect = view_rect;
    sprite_sheet_info_.source_rect = source_rect;
    sprite_sheet_info_.spacing = spacing;

    animation_info_.columns = static_cast<int>(source_rect.w / view_rect.w);
    int rows = static_cast<int>(source_rect.h / view_rect.h);
    animation_info_.frame_count = animation_info_.columns * rows;

    SetScale(view_rect.w, view_rect.h);

    Reset();
}

void AnimatedObject::UpdateFrameRect() 
{
    if (!texture_) 
    {
        return;
    }

    int row = animation_info_.current_frame % animation_info_.columns;
    int col = animation_info_.current_frame / animation_info_.columns;

    sprite_sheet_info_.view_rect.x = sprite_sheet_info_.offset.x +
        (row * (sprite_sheet_info_.view_rect.w + sprite_sheet_info_.spacing.x));
    sprite_sheet_info_.view_rect.y = sprite_sheet_info_.offset.y +
        (col * (sprite_sheet_info_.view_rect.h + sprite_sheet_info_.spacing.y));
}