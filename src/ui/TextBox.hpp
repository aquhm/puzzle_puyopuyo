#pragma once

#include "../game/RenderableObject.hpp"
#include <string>
#include <memory>
#include <functional>
#include <SDL3/SDL.h>

class StringTexture;

enum class TextType {
    Unicode,
    UTF8,
    ANSI
};

class TextBox : public RenderableObject 
{
public:
    static constexpr int MAX_TEXT_LENGTH = 150;
    static constexpr float CURSOR_ALPHA_VELOCITY = 300.0f;

    static constexpr SDL_Color COLOR_BLACK{ 0, 0, 0, 0 };
    static constexpr SDL_Color COLOR_WHITE{ 255, 255, 255, 255 };
    static constexpr SDL_Color COLOR_SKY{ 0, 36, 57, 255 };
    static constexpr SDL_Color COLOR_NAVY{ 0, 47, 71, 255 };

public:
    TextBox();
    ~TextBox() override;

    TextBox(const TextBox&) = delete;
    TextBox& operator=(const TextBox&) = delete;
    TextBox(TextBox&&) = delete;
    TextBox& operator=(TextBox&&) = delete;

    virtual bool Init(float x, float y, float width, float height);
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;

    virtual void HandleEvent(const SDL_Event& event);

    void UpdateDrawBox();
    virtual void ClearContent();
    void SetEventReturn(std::function<bool()> action);

    void SetEventCallback(std::function<bool()> action)
    {
        SetEventReturn(std::move(action));
    }

    [[nodiscard]] const char* GetText(TextType type) const;
    [[nodiscard]] const wchar_t* GetTextW() const;
    [[nodiscard]] bool IsEmpty() const noexcept { return content_.empty(); }

private:
    void UpdateCursorAlpha(float deltaTime);
    void HandleTextInput(const SDL_Event& event);
    void HandleTextEditing(const SDL_Event& event);
    void HandleKeyDown(const SDL_Event& event);
    void InitializeRenderTarget(float width, float height);

    void ProcessBackspace();
    void ProcessReturn();
    void UpdateTextureStates();

protected:
    std::string content_;                    // UTF8 storage
    mutable std::string content_ansi_;       // ANSI storage for conversion
    std::wstring composition_text_;          // IME composition text
    char16_t korean_unicode_text_{ 0 };      // Korean Unicode composition

    std::unique_ptr<StringTexture> input_title_texture_;
    std::unique_ptr<StringTexture> content_texture_;
    std::unique_ptr<StringTexture> ime_composition_texture_;

    // Render target
    std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> target_render_input_box_texture_;
    SDL_FRect input_box_rect_{ 0.0f, 0.0f, 0.0f, 0.0f };

    SDL_FRect cursor_rect_{ 0.0f, 4.0f, 2.0f, 15.0f };
    SDL_FRect composition_rect_{ 0.0f, 4.0f, 15.0f, 15.0f };
    SDL_FRect title_rect_{ 0.0f, 0.0f, 0.0f, 0.0f };

    float content_pos_{ 0.0f };
    float alpha_{ 255.0f };
    bool is_alpha_increasing_{ false };

    std::function<bool()> event_action_;
};