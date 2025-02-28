#include "TextBox.hpp"
#include "../texture/StringTexture.hpp"
#include "../core/GameApp.hpp"
#include "../utils/StringUtils.hpp"
#include "../utils/Logger.hpp"

#include <stdexcept>
#include <format>
#include <cassert>

TextBox::TextBox()
    : RenderableObject()
    , target_render_input_box_texture_(nullptr, SDL_DestroyTexture)
{
}

TextBox::~TextBox() = default;

bool TextBox::Init(float x, float y, float width, float height)
{
    try 
    {
        SetPosition(x, y);
        SetScale(width, height);

        input_title_texture_ = std::make_unique<StringTexture>();
        ime_composition_texture_ = std::make_unique<StringTexture>();
        content_texture_ = std::make_unique<StringTexture>();

        cursor_rect_ = { 0.0f, 4.0f, 2.0f, 15.0f };
        composition_rect_ = { 0.0f, 4.0f, 15.0f, 15.0f };

        title_rect_ = { x, y,
            input_title_texture_->GetWidth(), height };

        InitializeRenderTarget(width, height);
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize TextBox: %s", e.what());
        return false;
    }
}

void TextBox::InitializeRenderTarget(float width, float height)
{
    auto* renderer = GAME_APP.GetRenderer();
    if (!renderer) 
    {
        throw std::runtime_error("Renderer not available");
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        static_cast<int>(width - input_title_texture_->GetWidth()),
        static_cast<int>(height)
    );

    if (!texture) 
    {
        throw std::runtime_error(std::format("Failed to create render target texture: {}", SDL_GetError()));
    }

    target_render_input_box_texture_.reset(texture);
    SDL_SetTextureBlendMode(target_render_input_box_texture_.get(),
        SDL_BLENDMODE_BLEND);

    input_box_rect_ = {
        GetX() + input_title_texture_->GetWidth(),
        GetY(),
        width - input_title_texture_->GetWidth(),
        height
    };
}

void TextBox::Update(float deltaTime)
{
    UpdateCursorAlpha(deltaTime);
}

void TextBox::UpdateCursorAlpha(float deltaTime)
{
    constexpr float MAX_ALPHA = 255.0f;
    constexpr float MIN_ALPHA = 0.0f;

    if (is_alpha_increasing_) {
        alpha_ += (deltaTime * CURSOR_ALPHA_VELOCITY);
        if (alpha_ > MAX_ALPHA) {
            alpha_ = MAX_ALPHA;
            is_alpha_increasing_ = false;
        }
    }
    else {
        alpha_ -= (deltaTime * CURSOR_ALPHA_VELOCITY);
        if (alpha_ < MIN_ALPHA) {
            alpha_ = MIN_ALPHA;
            is_alpha_increasing_ = true;
        }
    }
}

void TextBox::Render()
{
    if (!IsVisible()) {
        return;
    }

    auto* renderer = GAME_APP.GetRenderer();
    if (!renderer) {
        return;
    }

    // Render title background
    SDL_SetRenderDrawColor(renderer, COLOR_NAVY.r, COLOR_NAVY.g, COLOR_NAVY.b, COLOR_NAVY.a);
    SDL_RenderFillRect(renderer, &title_rect_);

    // Render title text
    if (input_title_texture_) {
        input_title_texture_->Render(GetX(), GetY() + 2.0f);
    }

    // Switch to input box render target
    SDL_SetRenderTarget(renderer, target_render_input_box_texture_.get());
    SDL_SetRenderDrawColor(renderer, COLOR_NAVY.r, COLOR_NAVY.g, COLOR_NAVY.b, COLOR_NAVY.a);
    SDL_RenderClear(renderer);

    // Render cursor or composition text
    const auto alpha_u8 = static_cast<uint8_t>(alpha_);
    SDL_SetRenderDrawColor(renderer, alpha_u8, alpha_u8, alpha_u8, alpha_u8);

    if (korean_unicode_text_) {
        SDL_RenderFillRect(renderer, &composition_rect_);
    }
    else {
        SDL_RenderFillRect(renderer, &cursor_rect_);
    }

    // Render IME composition text
    if (ime_composition_texture_) {
        ime_composition_texture_->Render(
            content_texture_->GetWidth(),
            2.0f
        );
    }

    // Render main content
    if (content_texture_) {
        content_texture_->Render(content_pos_, 2.0f);
    }

    // Switch back to main render target and render the input box
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderTexture(renderer,
        target_render_input_box_texture_.get(),
        nullptr,
        &input_box_rect_
    );
}

void TextBox::UpdateDrawBox()
{
    if (!content_texture_) 
    {
        return;
    }

    try 
    {
        // Update content texture
        content_texture_->RenderText(content_, COLOR_WHITE);
        cursor_rect_.x = content_texture_->GetWidth();

        // Calculate content position for horizontal scrolling
        const float available_width = input_box_rect_.w - cursor_rect_.w;
        if (available_width < content_texture_->GetWidth()) {
            content_pos_ = available_width - content_texture_->GetWidth();
            cursor_rect_.x = content_texture_->GetWidth() + content_pos_;
        }
        else {
            content_pos_ = 0.0f;
            cursor_rect_.x = content_texture_->GetWidth();
        }

        // Update IME composition
        if (ime_composition_texture_ && !composition_text_.empty()) {
            ime_composition_texture_->RenderUnicode(
                reinterpret_cast<const char16_t*>(composition_text_.c_str()),
                COLOR_WHITE
            );
            composition_rect_.x = content_texture_->GetWidth();

            if (available_width < content_texture_->GetWidth()) {
                content_pos_ -= ime_composition_texture_->GetWidth();
                composition_rect_.x = content_texture_->GetWidth() + content_pos_;
            }
        }
    }
    catch (const std::exception& e) {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION,
            "Failed to update text box: %s", e.what());
    }
}

void TextBox::Release()
{
    input_title_texture_.reset();
    content_texture_.reset();
    ime_composition_texture_.reset();
    target_render_input_box_texture_.reset();
    event_action_ = nullptr;
}

void TextBox::SetEventReturn(std::function<bool()> action)
{
    event_action_ = std::move(action);
}

void TextBox::ClearContent()
{
    content_.clear();
    if (content_texture_) {
        content_texture_->Release();
    }
    UpdateDrawBox();
}

void TextBox::HandleEvent(const SDL_Event& event)
{
    switch (event.type) 
    {
    case SDL_EVENT_KEY_DOWN:
        HandleKeyDown(event);
        break;
    case SDL_EVENT_TEXT_INPUT:
        HandleTextInput(event);
        break;
    case SDL_EVENT_TEXT_EDITING:
        HandleTextEditing(event);
        break;
    }
}

void TextBox::HandleKeyDown(const SDL_Event& event)
{
    switch (event.key.key) 
    {
    case SDLK_RETURN:
        ProcessReturn();
        break;
    case SDLK_BACKSPACE:
        ProcessBackspace();
        break;
    }
}

void TextBox::ProcessReturn()
{
    if (content_.empty() || !composition_text_.empty()) 
    {
        return;
    }

    if (!composition_text_.empty() && korean_unicode_text_) 
    {
        content_.append(StringUtils::WideToUtf8(composition_text_));
        composition_text_.clear();
        korean_unicode_text_ = 0;
    }

    if (event_action_) {
        event_action_();
    }

    ClearContent();
}

void TextBox::ProcessBackspace()
{
    if (korean_unicode_text_ != 0) {
        return;
    }

    if (!content_.empty()) {
        // UTF-8 안전한 문자 제거
        auto iter = content_.end();
        --iter;
        while (iter != content_.begin() && (*iter & 0xC0) == 0x80) {
            --iter;
        }
        content_.erase(iter, content_.end());

        if (content_.empty() && content_texture_) {
            content_texture_->Release();
        }

        UpdateDrawBox();
    }
}

void TextBox::HandleTextInput(const SDL_Event& event)
{
    if (event.text.text[0] == '\0' || event.text.text[0] == '\n') {
        return;
    }

    if (content_.length() + SDL_strlen(event.text.text) < MAX_TEXT_LENGTH) {
        korean_unicode_text_ = 0;
        composition_text_.clear();

        if (ime_composition_texture_) {
            ime_composition_texture_->Release();
        }

        content_.append(event.text.text);
        UpdateDrawBox();
    }
}

void TextBox::HandleTextEditing(const SDL_Event& event)
{
    composition_text_ = StringUtils::Utf8ToWide(event.text.text);

    if (!composition_text_.empty()) 
    {
        char16_t* str = const_cast<char16_t*>(reinterpret_cast<const char16_t*>(composition_text_.c_str()));

        constexpr int HANGUL_BASE = 0xAC00;
        constexpr int JAMO_PER_SYLLABLE = 28;
        constexpr int JAMO_PER_CHOSUNG = 21 * JAMO_PER_SYLLABLE;

        int unicode = *str - HANGUL_BASE;
        int chosung = unicode / JAMO_PER_CHOSUNG;
        int jungsung = (unicode % JAMO_PER_CHOSUNG) / JAMO_PER_SYLLABLE;
        int jongsung = unicode % JAMO_PER_SYLLABLE;

        korean_unicode_text_ = static_cast<char16_t>(
            HANGUL_BASE +
            (chosung * JAMO_PER_CHOSUNG) +
            (jungsung * JAMO_PER_SYLLABLE) +
            jongsung
            );

        UpdateDrawBox();
    }
    else 
    {
        korean_unicode_text_ = 0;
        if (ime_composition_texture_) 
        {
            ime_composition_texture_->Release();
        }
    }
}

// GetText() 수정
const char* TextBox::GetText(TextType type) const
{
    switch (type) 
    {
    case TextType::UTF8:
        return content_.c_str();
    case TextType::ANSI:
        content_ansi_ = StringUtils::WideToAnsi(StringUtils::Utf8ToWide(content_));
        return content_ansi_.c_str();
    default:
        assert(false && "Invalid text type");
        return nullptr;
    }
}

// GetTextW() 수정
const wchar_t* TextBox::GetTextW() const
{
    static std::wstring temp = StringUtils::Utf8ToWide(content_);
    return temp.c_str();
}

void TextBox::UpdateTextureStates()
{
    // Update content texture if needed
    if (!content_.empty() && content_texture_) 
    {
        bool needsUpdate = !content_texture_->IsLoaded();
        if (needsUpdate) 
        {
            content_texture_->RenderText(content_, COLOR_WHITE);
        }
    }

    // Update IME composition texture if needed
    if (!composition_text_.empty() && ime_composition_texture_) 
    {
        bool needsUpdate = !ime_composition_texture_->IsLoaded();
        if (needsUpdate) 
        {
            ime_composition_texture_->RenderUnicode(reinterpret_cast<const char16_t*>(composition_text_.c_str()), COLOR_WHITE);
        }
    }

    // Update cursor and composition rectangles
    if (content_texture_) 
    {
        cursor_rect_.x = content_texture_->GetWidth();
        composition_rect_.x = cursor_rect_.x;

        // Adjust for horizontal scrolling if needed
        const float available_width = GetWidth() - input_title_texture_->GetWidth();
        
        if (available_width < content_texture_->GetWidth()) 
        {
            float offset = available_width - content_texture_->GetWidth();
            content_pos_ = offset;
            cursor_rect_.x += offset;
            composition_rect_.x += offset;
        }
    }
}