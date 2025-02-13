//#include "StringTexture.hpp"
//#include "../core/manager/FontManager.hpp"
//#include "../core/GameApp.hpp"
//#include <format>
//
//bool StringTexture::RenderText(
//    std::string_view text,
//    const SDL_Color& textColor,
//    StringEncoding encoding,
//    FontType fontType)
//{
//    if (text.empty()) 
//    {
//        return false;
//    }
//
//    try {
//        // ���� �ؽ�ó ����
//        Unload();
//
//        // ��Ʈ �Ŵ������� ��Ʈ ��������
//        auto* fontManager = GAME_APP.GetManager<FontManager>("FontManager");
//        if (!fontManager) {
//            throw std::runtime_error("Font manager not available");
//        }
//
//        TTF_Font* font = fontManager->GetFont(fontType);
//        if (!font) {
//            throw std::runtime_error("Failed to get font");
//        }
//
//        // SDL Surface ����
//        SDL_Surface* textSurface = nullptr;
//        if (encoding == StringEncoding::UTF8) {
//            textSurface = TTF_RenderUTF8_Blended(font,
//                std::string(text).c_str(), textColor);
//        }
//
//        if (!textSurface) {
//            throw std::runtime_error(
//                std::format("Failed to render text: {}", SDL_GetError()));
//        }
//
//        // Surface�κ��� �ؽ�ó ����
//        auto* renderer = GAME_APP.GetRenderer();
//        if (renderer == nullptr) 
//        {
//            SDL_DestroySurface(textSurface);
//            throw std::runtime_error("Renderer not available");
//        }
//
//        texture_ = SDL_CreateTextureFromSurface(renderer, textSurface);
//        if (!texture_) {
//            SDL_DestroySurface(textSurface);
//            throw std::runtime_error(
//                std::format("Failed to create texture: {}", SDL_GetError()));
//        }
//
//        // �ؽ�Ʈ ũ�� ����
//        if (encoding == StringEncoding::UTF8) {
//            if (TTF_SizeUTF8(font, std::string(text).c_str(),
//                &width_, &height_) != 0) {
//                throw std::runtime_error(
//                    std::format("Failed to get text size: {}", TTF_GetError()));
//            }
//        }
//
//        SDL_DestroySurface(textSurface);
//        encoding_ = encoding;
//        return true;
//    }
//    catch (const std::exception& e) {
//        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
//            "Failed to render text: %s", e.what());
//        return false;
//    }
//}
//
//bool StringTexture::RenderUnicode(const char16_t* text,
//    const SDL_Color& textColor,
//    FontType fontType)
//{
//    if (!text) {
//        return false;
//    }
//
//    try {
//        Unload();
//
//        auto* fontManager = GAME_APP.GetManager<FontManager>("FontManager");
//        if (!fontManager) {
//            throw std::runtime_error("Font manager not available");
//        }
//
//        TTF_Font* font = fontManager->GetFont(fontType);
//        if (!font) {
//            throw std::runtime_error("Failed to get font");
//        }
//
//        SDL_Surface* textSurface = TTF_RenderUNICODE_Blended(font,
//            reinterpret_cast<const Uint16*>(text), textColor);
//
//        if (!textSurface) {
//            throw std::runtime_error(
//                std::format("Failed to render Unicode text: {}", TTF_GetError()));
//        }
//
//        auto* renderer = GAME_APP.GetSDLRenderer();
//        if (!renderer) {
//            SDL_DestroySurface(textSurface);
//            throw std::runtime_error("Renderer not available");
//        }
//
//        texture_ = SDL_CreateTextureFromSurface(renderer, textSurface);
//        if (!texture_) {
//            SDL_DestroySurface(textSurface);
//            throw std::runtime_error(
//                std::format("Failed to create texture: {}", SDL_GetError()));
//        }
//
//        if (TTF_SizeUNICODE(font, reinterpret_cast<const Uint16*>(text),
//            &width_, &height_) != 0) {
//            throw std::runtime_error(
//                std::format("Failed to get text size: {}", TTF_GetError()));
//        }
//
//        SDL_DestroySurface(textSurface);
//        encoding_ = StringEncoding::Unicode;
//        return true;
//    }
//    catch (const std::exception& e) {
//        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
//            "Failed to render Unicode text: %s", e.what());
//        return false;
//    }
//}