//#pragma once
//#include "ImageTexture.hpp"
//#include <string>
//#include <string_view>
//#include <SDL3/SDL_ttf.h>
//
//// 문자열 텍스처의 인코딩 타입
//enum class StringEncoding {
//    UTF8,
//    Unicode,
//};
//
//class StringTexture : public ImageTexture {
//public:
//    StringTexture() = default;
//    ~StringTexture() override = default;
//
//    // 텍스트 렌더링 메서드
//    [[nodiscard]] bool RenderText(std::string_view text,
//        const SDL_Color& textColor,
//        StringEncoding encoding = StringEncoding::UTF8,
//        FontType fontType = FontType::Chat);
//
//    // UTF8 전용 렌더링 (편의 메서드)
//    [[nodiscard]] bool RenderUTF8(std::string_view text,
//        const SDL_Color& textColor,
//        FontType fontType = FontType::Chat) {
//        return RenderText(text, textColor, StringEncoding::UTF8, fontType);
//    }
//
//    // Unicode 전용 렌더링 (편의 메서드)
//    [[nodiscard]] bool RenderUnicode(const char16_t* text,
//        const SDL_Color& textColor,
//        FontType fontType = FontType::Chat);
//
//private:
//    StringEncoding encoding_{ StringEncoding::UTF8 };
//};