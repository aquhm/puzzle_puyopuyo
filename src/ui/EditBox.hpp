#pragma once
#include "TextBox.hpp"
#include <deque>
#include <memory>
#include <chrono>

class StringTexture;

struct ViewText {
    std::unique_ptr<StringTexture> message;
    float accumTime{ 0.0f };
    float alpha{ 255.0f };
};

class EditBox : public TextBox {
public:
    static constexpr float TEXT_VIEW_LIMIT_TIME = 10.0f;
    static constexpr float FADE_OUT_SPEED = 50.0f;

public:
    EditBox();
    ~EditBox() override;

    // Delete copy operations
    EditBox(const EditBox&) = delete;
    EditBox& operator=(const EditBox&) = delete;

    // Core functionality
    bool Init(float x, float y, float width, float height) override;
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    // Event handling
    void HandleEvent(const SDL_Event& event) override;

    // Content management
    void InputContent(std::string_view text);
    void ClearContent() override;

private:
    std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> render_target_texture_;
    SDL_FRect render_target_rect_{ 0.0f, 0.0f, 0.0f, 0.0f };

    std::deque<std::unique_ptr<ViewText>> message_list_;

    // Helper methods
    void RenderMessageList();
    void UpdateMessageList(float deltaTime);
    void CreateRenderTarget();
    void InitializeRenderTargets(float width, float height);

    [[nodiscard]] bool IsMessageListEmpty() const { return message_list_.empty(); }
    void ProcessTextInput(std::string_view text);
};