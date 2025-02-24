#pragma once

#include "../utils/PathUtil.hpp"
#include <SDL3/SDL_ttf.h>

class ImageTexture;
class Font;

template<typename T>
struct ResourcePathTrait;

template<>
struct ResourcePathTrait<ImageTexture> 
{
    static constexpr const char* BaseDir = PathUtil::IMAGE_DIR;
};

template<>
struct ResourcePathTrait<TTF_Font> {
    static constexpr const char* BaseDir = PathUtil::FONT_DIR;
};

