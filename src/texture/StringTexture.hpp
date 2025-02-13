//#pragma once
//#include "ImageTexture.hpp"
//#include <string>
//#include <string_view>
//#include <SDL3/SDL_ttf.h>
//
//// ���ڿ� �ؽ�ó�� ���ڵ� Ÿ��
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
//    // �ؽ�Ʈ ������ �޼���
//    [[nodiscard]] bool RenderText(std::string_view text,
//        const SDL_Color& textColor,
//        StringEncoding encoding = StringEncoding::UTF8,
//        FontType fontType = FontType::Chat);
//
//    // UTF8 ���� ������ (���� �޼���)
//    [[nodiscard]] bool RenderUTF8(std::string_view text,
//        const SDL_Color& textColor,
//        FontType fontType = FontType::Chat) {
//        return RenderText(text, textColor, StringEncoding::UTF8, fontType);
//    }
//
//    // Unicode ���� ������ (���� �޼���)
//    [[nodiscard]] bool RenderUnicode(const char16_t* text,
//        const SDL_Color& textColor,
//        FontType fontType = FontType::Chat);
//
//private:
//    StringEncoding encoding_{ StringEncoding::UTF8 };
//};