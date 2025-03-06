#pragma once

#include "../game/RenderableObject.hpp"
#include <functional>
#include <memory>
#include <array>

struct SDL_FRect;
union SDL_Event;

class ImageTexture;

class Button : public RenderableObject 
{
public:
    enum class State 
    {
        Normal,
        Hover,
        Down,
        Up,
        Max
    };

    enum class TextureState : uint8_t 
    {
        None = 0,
        Normal = 1 << 0,
        Hover = 1 << 1,
        Down = 1 << 2,
        Up = 1 << 3
    };

    using EventCallback = std::function<bool()>;

    Button() noexcept;
    ~Button() override;

    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    Button(Button&&) noexcept = default;
    Button& operator=(Button&&) noexcept = default;

    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    void Init(std::shared_ptr<ImageTexture> texture, float x, float y, float width, float height);
    void HandleEvent(const SDL_Event& event) noexcept;

    bool SetStateRect(State state, const SDL_FRect& rect) noexcept;
    bool SetEventCallback(State state, EventCallback callback);

    [[nodiscard]] State GetCurrentState() const noexcept { return current_state_; }
    [[nodiscard]] TextureState GetTextureState() const noexcept { return texture_state_; }

private:

    [[nodiscard]] bool IsPointInside(float x, float y) const noexcept;
    void UpdateButtonState(const SDL_Event& event) noexcept;

private:
    std::shared_ptr<ImageTexture> texture_;
    std::array<EventCallback, static_cast<size_t>(State::Max)> callbacks_;
    std::array<SDL_FRect, 4> state_rects_;

    State current_state_{ State::Normal };
    TextureState texture_state_{ TextureState::None };

};

constexpr Button::TextureState operator|(Button::TextureState a, Button::TextureState b) noexcept 
{
    return static_cast<Button::TextureState>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr bool operator&(Button::TextureState a, Button::TextureState b) noexcept 
{
    return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
}