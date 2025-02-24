#pragma once

#include "IManager.hpp"
#include <vector>
#include <memory>
#include <string_view>
#include <filesystem>
#include <SDL3/SDL_ttf.h>


// 폰트 타입을 enum class로 정의
enum class FontType 
{
    Chat,
    Notice,
    UI,
    Count
};

class FontManager final : public IManager 
{
public:

    FontManager() = default;
    ~FontManager() override;

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    [[nodiscard]] std::string_view GetName() const override { return "FontManager"; }
    [[nodiscard]] TTF_Font* GetFont(FontType type) const;
    void LoadFont(FontType type, const std::string& filename, float size);

private:

    [[nodiscard]] bool IsValidFontType(FontType type) const;
    void ValidateFontInitialization();

private:

    friend class Managers;
    
    struct FontDeleter 
    {
        void operator()(TTF_Font* font) const;
    };

    std::vector<std::unique_ptr<TTF_Font, FontDeleter>> fonts_;
    
};