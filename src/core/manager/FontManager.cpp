#include "FontManager.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <format>
#include <stdexcept>

void FontManager::FontDeleter::operator()(TTF_Font* font) const
{
    if (font) 
    {
        TTF_CloseFont(font);
    }
}

FontManager::~FontManager() 
{
    Release();
}

bool FontManager::Initialize() 
{
    try 
    {
        ValidateFontInitialization();
        InitializeFontContainer();
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Font initialization failed: %s", e.what());
        return false;
    }
}

void FontManager::Update(float deltaTime) 
{
    // 폰트 매니저는 프레임별 업데이트가 필요 없음
    (void)deltaTime;
}

void FontManager::Release() 
{
    fonts_.clear();  // unique_ptr가 자동으로 폰트 리소스 정리
    TTF_Quit();
}

TTF_Font* FontManager::GetFont(FontType type) const
{
    if (!IsValidFontType(type)) 
    {
        return nullptr;
    }

    auto index = static_cast<size_t>(type);
    return fonts_[index].get();
}

bool FontManager::LoadFont(FontType type, const std::filesystem::path& path, int size) 
{
    if (!IsValidFontType(type)) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid font type");
        return false;
    }

    try {
        auto* font = TTF_OpenFont(path.string().c_str(), size);
        
        if (font == nullptr) 
        {
            throw std::runtime_error(std::format("Failed to load font {}: {}", path.string(), SDL_GetError()));
        }

        auto index = static_cast<size_t>(type);
        fonts_[index].reset(font);
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Font loading failed: %s", e.what());
        return false;
    }
}

void FontManager::InitializeFontContainer() 
{
    fonts_.resize(static_cast<size_t>(FontType::Count));
}

bool FontManager::IsValidFontType(FontType type) const 
{
    return static_cast<size_t>(type) < static_cast<size_t>(FontType::Count);
}

void FontManager::ValidateFontInitialization() 
{
    if (TTF_Init() < 0) 
    {
        throw std::runtime_error(std::format("SDL_ttf initialization failed: {}", SDL_GetError()));
    }
}