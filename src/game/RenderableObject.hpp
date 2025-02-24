#pragma once
/*
 *
 * 설명: 화면상에 그려지는 객체에 대한 추상 Class
 *
 */

#include <SDL3/SDL.h>
#include "../core/IRenderable.hpp"
#include "../math/Vector2.h"

class RenderableObject : public IRenderable 
{
public:
    RenderableObject() = default;
    ~RenderableObject() override = default;

    // Delete copy/move operations to prevent slicing
    RenderableObject(const RenderableObject&) = delete;
    RenderableObject& operator=(const RenderableObject&) = delete;
    RenderableObject(RenderableObject&&) = delete;
    RenderableObject& operator=(RenderableObject&&) = delete;

    // Core functionality
    virtual void Update(float deltaTime) = 0;
    void Render() override = 0;
    virtual void Release() = 0;

    // Position methods
    virtual void SetPosition(float x, float y);
    virtual void SetX(float x);
    virtual void SetY(float y);

    // Size methods
    virtual void SetSize(float width, float height);

    // Getters
    [[nodiscard]] float GetX() const { return destination_rect_.x; }
    [[nodiscard]] float GetY() const { return destination_rect_.y; }
    [[nodiscard]] float GetWidth() const { return destination_rect_.w; }
    [[nodiscard]] float GetHeight() const { return destination_rect_.h; }
    [[nodiscard]] const SDL_FRect& GetRect() const { return destination_rect_; }
    [[nodiscard]] bool IsVisible() const { return is_visible_; }

    [[nodiscard]] SDL_FPoint GetPosition() const { return position_; }
    [[nodiscard]] SDL_FPoint GetSize() const { return size_; }

    // Visibility control
    void SetVisible(bool visible) { is_visible_ = visible; }

protected:
	
    SDL_FPoint position_{ 0.0f, 0.0f };
    SDL_FPoint size_{ 0.0f, 0.0f };

    SDL_FRect destination_rect_{ 0.0f, 0.0f, 0.0f, 0.0f };
    bool is_visible_{ true };

    virtual void UpdateDestinationRect() 
    {
        destination_rect_.x = position_.x;
        destination_rect_.y = position_.y;
        destination_rect_.w = size_.x;
        destination_rect_.h = size_.y;
    }
};