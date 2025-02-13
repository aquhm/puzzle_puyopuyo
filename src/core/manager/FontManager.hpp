#pragma once

#include "IManager.hpp"
#include <vector>
#include <memory>
#include <string_view>
#include <filesystem>


// ��Ʈ Ÿ���� enum class�� ����
enum class FontType 
{
    Chat,
    Notice,
    UI,
    Count  // enum�� ũ�⸦ ��� ���� ��Ŀ
};

class FontManager final : public IManager 
{
public:

    // Delete copy/move
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

    // IManager interface implementation
    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    [[nodiscard]] std::string_view GetName() const override { return "FontManager"; }

    // Font management
    [[nodiscard]] TTF_Font* GetFont(FontType type) const;
    [[nodiscard]] bool LoadFont(FontType type, const std::filesystem::path& path, int size);

private:
    friend class Managers;
    FontManager() = default;
    ~FontManager() override;

    // Font deleter for RAII
    struct FontDeleter 
    {
        void operator()(TTF_Font* font) const;
    };

    // Font container
    std::vector<std::unique_ptr<TTF_Font, FontDeleter>> fonts_;

    // Helper methods
    void InitializeFontContainer();
    [[nodiscard]] bool IsValidFontType(FontType type) const;
    static void ValidateFontInitialization();
};