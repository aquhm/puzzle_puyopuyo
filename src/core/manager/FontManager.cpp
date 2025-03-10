#include "FontManager.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <format>
#include <stdexcept>
#include <cassert>
#include "../../utils/PathUtil.hpp"
#include "../../utils/Logger.hpp"

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
        fonts_.resize(static_cast<size_t>(FontType::Count));

        ValidateFontInitialization();        

        LoadFont(FontType::Chat, "NanumGothicBold.ttf", 15);
        return true;

    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Font initialization failed: %s", e.what());
        return false;
    }
}

void FontManager::Update(float deltaTime) 
{
}

void FontManager::Release() 
{
    fonts_.clear();
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

void FontManager::LoadFont(FontType type, const std::string& filename, float size) 
{
    if (!IsValidFontType(type)) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Invalid font type");
        return;
    }

    try 
    {
        std::string exeBasePath = PathUtil::GetExecutableBasePath();
        std::string fontPath = PathUtil::CombinePaths(exeBasePath, PathUtil::CombinePaths(PathUtil::FONT_DIR, filename));
        std::string fullPath = PathUtil::CombinePaths(exeBasePath, fontPath);
        
        auto font = TTF_OpenFont(fullPath.c_str(), size);
        if (font == nullptr) 
        {
            throw std::runtime_error(std::format("Failed to load font {}: {}", fullPath, SDL_GetError()));
        }

        auto index = static_cast<size_t>(type);
        fonts_[index].reset(font);
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Font loading failed: %s", e.what());
    }
}


bool FontManager::IsValidFontType(FontType type) const 
{
    return static_cast<size_t>(type) < static_cast<size_t>(FontType::Count);
}

void FontManager::ValidateFontInitialization() 
{
    if (TTF_Init() == false) 
    {
        throw std::runtime_error(std::format("SDL_ttf initialization failed: {}", SDL_GetError()));
    }
}