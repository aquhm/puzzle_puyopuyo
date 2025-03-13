#pragma once
/*
 *
 * 설명: Frame 별 Sprite 표시하여 애니메이션 효과를 나타내는 Object Class
 *
 */

#include "RenderableObject.hpp"
#include "../texture/ImageTexture.hpp"

#include <memory>

class ImgTexture;

class AnimatedObject : public RenderableObject 
{
public:
    struct AnimationInfo 
    {
        int frame_count{ 0 };          
        int current_frame{ 0 };        
        int columns{ 0 };              
        float frame_time{ 0.0f };      
        float accumulated_time{ 0.0f };
        bool is_looping{ true };       
        bool is_playing{ false };      
        bool is_reverse{ false };      
    };

    struct SpriteSheetInfo 
    {
        SDL_FRect view_rect{};        
        SDL_FRect source_rect{};      
        SDL_FPoint spacing{ 0, 0 };   
        SDL_FPoint offset{ 0, 0 };    
    };

    AnimatedObject() = default;
    ~AnimatedObject() override = default;

    // Core functionality
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    // Animation control
    void Play();
    void Stop();
    void Pause();
    void Reset();
    void SetReverse(bool reverse);

    // Texture setup
    void SetTextureInfo(std::shared_ptr<ImageTexture> texture,
        const SDL_FRect& view_rect,
        const SDL_FRect& source_rect,
        const SDL_FPoint& spacing);

    // Animation parameters
    void SetFrameTime(float time) { animation_info_.frame_time = time; }
    void SetLooping(bool loop) { animation_info_.is_looping = loop; }

    // State queries
    [[nodiscard]] bool IsPlaying() const { return animation_info_.is_playing; }
    [[nodiscard]] bool IsReverse() const { return animation_info_.is_reverse; }
    [[nodiscard]] int GetCurrentFrame() const { return animation_info_.current_frame; }
    [[nodiscard]] const SDL_FRect& GetViewRect() const { return sprite_sheet_info_.view_rect; }

    void UpdateFrameRect();

protected:
    std::shared_ptr<ImageTexture> texture_;
    AnimationInfo animation_info_;
    SpriteSheetInfo sprite_sheet_info_;
};