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

    // Delete copy operations to prevent accidental copies
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;

    // Allow move operations
    Button(Button&&) noexcept = default;
    Button& operator=(Button&&) noexcept = default;

    // Core functionality from RenderableObject
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    // Modern initialization with strong exception guarantees
    void Init(std::shared_ptr<ImageTexture> texture, float x, float y, float width, float height);

    // Event handling
    void HandleEvent(const SDL_Event& event) noexcept;

    // State management
    bool SetStateRect(State state, const SDL_FRect& rect) noexcept;
    bool SetEventCallback(State state, EventCallback callback);

    // Getters
    [[nodiscard]] State GetCurrentState() const noexcept { return current_state_; }
    [[nodiscard]] TextureState GetTextureState() const noexcept { return texture_state_; }

private:
    std::shared_ptr<ImageTexture> texture_;
    std::array<EventCallback, static_cast<size_t>(State::Max)> callbacks_;
    std::array<SDL_FRect, 4> state_rects_;

    State current_state_{ State::Normal };
    TextureState texture_state_{ TextureState::None };

    // Utility functions
    [[nodiscard]] bool IsPointInside(float x, float y) const noexcept;
    void UpdateButtonState(const SDL_Event& event) noexcept;
};

// Operator overloads for TextureState flags
// Operator overloads for TextureState flags
constexpr Button::TextureState operator|(Button::TextureState a, Button::TextureState b) noexcept 
{
    return static_cast<Button::TextureState>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr bool operator&(Button::TextureState a, Button::TextureState b) noexcept 
{
    return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
}