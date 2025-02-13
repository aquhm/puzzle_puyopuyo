#pragma once

#include <SDL3/SDL.h>
#include "../core/IRenderable.hpp"

class RenderableObject : public IRenderable {
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

    // Visibility control
    void SetVisible(bool visible) { is_visible_ = visible; }

protected:
    SDL_FRect destination_rect_{ 0.0f, 0.0f, 0.0f, 0.0f };
    bool is_visible_{ true };
};