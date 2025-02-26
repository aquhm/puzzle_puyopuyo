#include "EditBox.hpp"
#include "../texture/StringTexture.hpp"
#include "../core/GameApp.hpp"
#include "../utils/StringUtils.hpp"
#include "../core/manager/FontManager.hpp"

#include <format>
#include <stdexcept>
#include "../utils/Logger.hpp"

EditBox::EditBox()
    : TextBox()
    , render_target_texture_(nullptr, SDL_DestroyTexture)
{
}

EditBox::~EditBox() = default;

bool EditBox::Init(float x, float y, float width, float height)
{
    try 
    {
        if (!TextBox::Init(x, y, width, height)) 
        {
            return false;
        }

        if (input_title_texture_) 
        {
            auto title_text = StringUtils::Utf8ToWide("[입력]");
            input_title_texture_->RenderText(StringUtils::WideToUtf8(title_text), COLOR_WHITE);
        }

        InitializeRenderTargets(width, height);
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize EditBox: %s", e.what());
        return false;
    }
}

void EditBox::InitializeRenderTargets(float width, float height)
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
        static_cast<int>(width),
        170  // Fixed height for message view area
    );

    if (!texture) {
        throw std::runtime_error(
            std::format("Failed to create render target texture: {}", SDL_GetError()));
    }

    render_target_texture_.reset(texture);
    SDL_SetTextureBlendMode(render_target_texture_.get(), SDL_BLENDMODE_BLEND);

    render_target_rect_ = {
        GetX(),
        200.0f,  // Fixed Y position for message view area
        width,
        170.0f
    };
}

void EditBox::Update(float deltaTime)
{
    TextBox::Update(deltaTime);
    UpdateMessageList(deltaTime);
}

void EditBox::UpdateMessageList(float deltaTime)
{
    for (auto it = message_list_.begin(); it != message_list_.end();) {
        auto& view_text = *it;

        view_text->accumTime += deltaTime;

        if (view_text->accumTime > TEXT_VIEW_LIMIT_TIME) {
            // Start fading out
            view_text->alpha -= (deltaTime * FADE_OUT_SPEED);

            if (view_text->alpha <= 0.0f) {
                // Remove completely faded out messages
                it = message_list_.erase(it);
            }
            else {
                // Update alpha of the texture
                if (view_text->message) {
                    view_text->message->SetAlpha(
                        static_cast<uint8_t>(view_text->alpha));
                }
                ++it;
            }
        }
        else {
            ++it;
        }
    }
}

void EditBox::Render()
{
    if (!IsVisible()) 
    {
        return;
    }

    TextBox::Render();
    RenderMessageList();
}

void EditBox::RenderMessageList()
{
    if (IsMessageListEmpty()) 
    {
        return;
    }

    auto* renderer = GAME_APP.GetRenderer();
    if (!renderer) 
    {
        return;
    }

    // Switch to message list render target
    SDL_SetRenderTarget(renderer, render_target_texture_.get());
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Render messages from bottom to top
    float y_position = render_target_rect_.h;
    constexpr float LINE_HEIGHT = 16.0f;

    for (auto it = message_list_.rbegin(); it != message_list_.rend(); ++it) 
    {
        const auto& view_text = *it;
        if (view_text->message) 
        {
            y_position -= LINE_HEIGHT;
            view_text->message->Render(0.0f, y_position);
        }
    }

    // Switch back to main render target
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderTexture(renderer,
        render_target_texture_.get(),
        nullptr,
        &render_target_rect_
    );
}

void EditBox::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN) 
    {
        switch (event.key.key) 
        {
        case SDLK_RETURN:
            if (!content_.empty() && composition_text_.empty()) 
            {
                if (!composition_text_.empty() && korean_unicode_text_) 
                {
                    content_.append(StringUtils::WideToUtf8(composition_text_));
                    composition_text_.clear();
                    korean_unicode_text_ = 0;
                }

                // Execute callback if set
                if (event_action_) 
                {
                    event_action_();
                }

                // Add to message list
                InputContent(content_);

                // Clear input
                content_.clear();
                if (content_texture_) 
                {
                    content_texture_->Release();
                }

                UpdateDrawBox();
            }
            break;
        default:
            TextBox::HandleEvent(event);
            break;
        }
    }
    else {
        TextBox::HandleEvent(event);
    }
}

void EditBox::InputContent(std::string_view text)
{
    if (text.empty()) 
    {
        return;
    }

    try 
    {
        auto view_text = std::make_unique<ViewText>();
        view_text->message = std::make_unique<StringTexture>();
        view_text->message->RenderText(text, COLOR_SKY);
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to add message: %s", e.what());
    }
}

void EditBox::Release()
{
    TextBox::Release();
    ClearContent();
    render_target_texture_.reset();
}

void EditBox::ClearContent()
{
    TextBox::ClearContent();
    message_list_.clear();
}

void EditBox::ProcessTextInput(std::string_view text)
{
    constexpr float MAX_WIDTH = 300.0f;  // Maximum width for a single line

    auto& font_manager = GAME_APP.GetFontManager();
    if (&font_manager == nullptr) 
    {
        return;
    }

    TTF_Font* font = font_manager.GetFont(FontType::Chat);
    if (!font) {
        return;
    }

    int text_width = 0;
    TTF_GetStringSize(font, std::string(text).c_str(), 0, &text_width, nullptr);

    if (text_width <= MAX_WIDTH) {
        // Text fits in one line
        InputContent(text);
    }
    else {
        // Text needs to be split into multiple lines
        std::string current_line;
        int current_width = 0;

        std::string utf8_text(text);
        size_t pos = 0;

        while (pos < utf8_text.length()) {
            size_t next_pos = pos;
            int char_width = 0;

            // Find next UTF-8 character boundary
            if ((utf8_text[pos] & 0x80) == 0) {
                next_pos += 1;  // ASCII character
            }
            else if ((utf8_text[pos] & 0xE0) == 0xC0) {
                next_pos += 2;  // 2-byte UTF-8
            }
            else if ((utf8_text[pos] & 0xF0) == 0xE0) {
                next_pos += 3;  // 3-byte UTF-8
            }
            else {
                next_pos += 4;  // 4-byte UTF-8
            }

            std::string char_str = utf8_text.substr(pos, next_pos - pos);
            TTF_GetStringSize(font, char_str.c_str(), 0, &char_width, nullptr);

            if (current_width + char_width > MAX_WIDTH) 
            {
                // Current line is full, create new message
                if (!current_line.empty()) {
                    InputContent(current_line);
                }
                current_line.clear();
                current_width = 0;
            }

            current_line += char_str;
            current_width += char_width;
            pos = next_pos;
        }

        // Add remaining text
        if (!current_line.empty()) 
        {
            InputContent(current_line);
        }
    }
}

void EditBox::CreateRenderTarget()
{
    auto* renderer = GAME_APP.GetRenderer();
    if (!renderer) 
    {
        throw std::runtime_error("Renderer not available");
    }

    // 이전 텍스처 해제
    render_target_texture_.reset();

    // 새 렌더 타겟 텍스처 생성
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        static_cast<int>(GetWidth()),
        static_cast<int>(render_target_rect_.h)
    );

    if (!texture) 
    {
        throw std::runtime_error(std::format("Failed to create render target texture: {}", SDL_GetError()));
    }

    render_target_texture_.reset(texture);

    // 블렌드 모드 설정
    SDL_SetTextureBlendMode(render_target_texture_.get(), SDL_BLENDMODE_BLEND);

    // 렌더 타겟 영역 업데이트
    render_target_rect_.x = GetX();
    render_target_rect_.w = GetWidth();
}