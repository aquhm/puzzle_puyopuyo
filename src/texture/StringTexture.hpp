#pragma once

#include "ImageTexture.hpp"
#include "../core/manager/FontManager.hpp"
#include <string>
#include <string_view>
#include <SDL3/SDL_ttf.h>
#include <memory>

struct SDL_Color;

enum class StringEncoding 
{
    UTF8,
    Unicode,
    Max
};

class StringTexture : public ImageTexture 
{
public:
    StringTexture() = default;
    ~StringTexture() override = default;

    StringTexture(const StringTexture&) = delete;
    StringTexture& operator=(const StringTexture&) = delete;
    StringTexture(StringTexture&&) noexcept = default;
    StringTexture& operator=(StringTexture&&) noexcept = default;

    void RenderText(std::string_view text, const SDL_Color& textColor, StringEncoding encoding = StringEncoding::UTF8, FontType fontType = FontType::Chat);
    void RenderUTF8(std::string_view text, const SDL_Color& textColor,FontType fontType = FontType::Chat) 
    {
        RenderText(text, textColor, StringEncoding::UTF8, fontType);
    }

    void RenderUnicode(std::u16string_view text,const SDL_Color& textColor,FontType fontType = FontType::Chat);

    void Release() 
    {
        Unload();
    }

private:
    [[nodiscard]] static SDL_Surface* CreateTextSurface(TTF_Font* font,
        std::string_view text,
        const SDL_Color& color,
        StringEncoding encoding);

private:
    StringEncoding encoding_{ StringEncoding::UTF8 };
    
};