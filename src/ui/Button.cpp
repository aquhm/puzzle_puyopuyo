#include "Button.hpp"
#include "../texture/ImageTexture.hpp"
#include <SDL3/SDL.h>
#include <cassert>
#include <stdexcept>

Button::Button() noexcept :
    RenderableObject{},
    texture_{ nullptr },
    callbacks_{},
    state_rects_{},
    current_state_{ State::Normal },
    texture_state_{ TextureState::None }
{
}

Button::~Button() = default;

void Button::Init(std::shared_ptr<ImageTexture> texture, float x, float y, float width, float height)
{
    texture_ = texture;
    if (!texture_) 
    {
        throw std::runtime_error("Button initialization failed: Invalid texture");
    }

    SetPosition(x, y);
    SetScale(width, height);
}

void Button::Update(float deltaTime)
{
}

void Button::Render()
{
    if (!IsVisible() || !texture_) {
        return;
    }

    auto idx = static_cast<size_t>(current_state_);
    switch (current_state_) {
    case State::Normal:
        if (!(texture_state_ & TextureState::Normal)) 
        {
            idx = static_cast<size_t>(State::Normal);
        }
        break;
    case State::Hover:
        if (!(texture_state_ & TextureState::Hover)) 
        {
            idx = static_cast<size_t>(State::Normal);
        }
        break;
    case State::Down:
        if (!(texture_state_ & TextureState::Down)) 
        {
            idx = static_cast<size_t>(State::Normal);
        }
        break;
    case State::Up:
        if (!(texture_state_ & TextureState::Up)) 
        {
            idx = static_cast<size_t>(State::Normal);
        }
        break;
    default:
        idx = static_cast<size_t>(State::Normal);
    }

    texture_->Render(GetX(), GetY(), &state_rects_[idx]);
}

void Button::HandleEvent(const SDL_Event& event) noexcept
{
    if (!IsVisible()) {
        return;
    }

    if (event.type == SDL_EVENT_MOUSE_MOTION ||
        event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        float mouse_x = 0, mouse_y = 0;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        if (!IsPointInside(mouse_x, mouse_y)) {
            current_state_ = State::Normal;
            return;
        }

        UpdateButtonState(event);
    }
}

bool Button::SetStateRect(State state, const SDL_FRect& rect) noexcept
{
    if (state >= State::Max) {
        assert(false && "Invalid button state");
        return false;
    }

    state_rects_[static_cast<size_t>(state)] = rect;

    switch (state) {
    case State::Normal:
        texture_state_ = texture_state_ | TextureState::Normal;
        break;
    case State::Hover:
        texture_state_ = texture_state_ | TextureState::Hover;
        break;
    case State::Down:
        texture_state_ = texture_state_ | TextureState::Down;
        break;
    case State::Up:
        texture_state_ = texture_state_ | TextureState::Up;
        break;
    default:
        break;
    }

    return true;
}

bool Button::SetEventCallback(State state, EventCallback callback)
{
    if (state >= State::Max) {
        assert(false && "Invalid button state");
        return false;
    }

    callbacks_[static_cast<size_t>(state)] = std::move(callback);
    return true;
}

void Button::Release()
{
    texture_.reset();
    callbacks_ = {};
    current_state_ = State::Normal;
    texture_state_ = TextureState::None;
}

bool Button::IsPointInside(float x, float y) const noexcept
{
    const auto& rect = GetRect();
    return (x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h);
}

void Button::UpdateButtonState(const SDL_Event& event) noexcept
{
    switch (event.type) 
    {
    case SDL_EVENT_MOUSE_MOTION:
        current_state_ = State::Hover;
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        current_state_ = State::Down;
        if (IsVisible() && callbacks_[static_cast<size_t>(current_state_)]) {
            callbacks_[static_cast<size_t>(current_state_)]();
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        current_state_ = State::Up;
        if (IsVisible() && callbacks_[static_cast<size_t>(current_state_)]) {
            callbacks_[static_cast<size_t>(current_state_)]();
        }
        break;

    default:
        break;
    }
}