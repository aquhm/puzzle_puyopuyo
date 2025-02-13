#include "ImageTexture.hpp"
#include "../core/manager/ResourceManager.hpp"
#include <SDL3/SDL_image.h>
#include <stdexcept>
#include <format>
#include "../core/GameApp.hpp"

ImageTexture::~ImageTexture() 
{
    ReleaseTexture();
}

ImageTexture::ImageTexture(ImageTexture&& other) noexcept
    : texture_(other.texture_)
    , path_(std::move(other.path_))
    , width_(other.width_)
    , height_(other.height_) 
{
    other.texture_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
}

ImageTexture& ImageTexture::operator=(ImageTexture&& other) noexcept 
{
    if (this != &other) 
    {
        ReleaseTexture();
        texture_ = other.texture_;
        path_ = std::move(other.path_);
        width_ = other.width_;
        height_ = other.height_;
        other.texture_ = nullptr;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

bool ImageTexture::Load(const std::string& path) 
{
    ReleaseTexture();
    path_ = path;

    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) 
    {
        throw std::runtime_error(std::format("Unable to load image {}: {}", path, SDL_GetError()));
    }

    auto formatDetail = SDL_GetPixelFormatDetails(loadedSurface->format);    
    if (formatDetail != nullptr)
    {
        auto colorKey = SDL_MapRGB(formatDetail, NULL, 0, 0, 0);
        SDL_SetSurfaceColorKey(loadedSurface, true, colorKey);
    }

    auto* renderer = GAME_APP.GetRenderer();
    if (renderer == nullptr) 
    {
        SDL_DestroySurface(loadedSurface);
        throw std::runtime_error("No renderer available");
    }

    texture_ = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    
    if (texture_) 
    {
        SDL_DestroySurface(loadedSurface);
        throw std::runtime_error(std::format("Unable to create texture: {}", SDL_GetError()));
    }

    width_ = loadedSurface->w;
    height_ = loadedSurface->h;

    SDL_DestroySurface(loadedSurface);
    return true;
}

void ImageTexture::Unload() 
{
    ReleaseTexture();
}

void ImageTexture::ReleaseTexture() 
{
    if (texture_ != nullptr)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
        width_ = 0;
        height_ = 0;
    }
}

void ImageTexture::SetColor(uint8_t red, uint8_t green, uint8_t blue) 
{
    if (texture_ != nullptr) 
    {
        SDL_SetTextureColorMod(texture_, red, green, blue);
    }
}

void ImageTexture::SetBlendMode(SDL_BlendMode blending) 
{
    if (texture_ != nullptr)
    {
        SDL_SetTextureBlendMode(texture_, blending);
    }
}

void ImageTexture::SetAlpha(uint8_t alpha) 
{
    if (texture_ != nullptr) 
    {
        SDL_SetTextureAlphaMod(texture_, alpha);
    }
}

void ImageTexture::Render(int x, int y, const SDL_FRect* sourceRect, double angle, const SDL_FPoint* center, SDL_FlipMode flip) const 
{
    if (texture_ == nullptr)
    {
        return;
    }

    auto* renderer = GAME_APP.GetRenderer();
    if (renderer == nullptr)
    {
        return;
    }

    SDL_FRect destRect{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(sourceRect ? sourceRect->w : width_),
        static_cast<float>(sourceRect ? sourceRect->h : height_)
    };

    SDL_RenderTextureRotated(renderer, texture_, sourceRect, &destRect, angle, center, flip);
}

void ImageTexture::RenderScaled(const SDL_FRect* sourceRect, const SDL_FRect* destRect,
    double angle, const SDL_FPoint* center,
    SDL_FlipMode flip) const 
{
    if (texture_ == nullptr)
    {
        return;
    }

    auto* renderer = GAME_APP.GetRenderer();
    if (renderer == nullptr)
    {
        return;
    }

    SDL_RenderTextureRotated(renderer, texture_, sourceRect, destRect, angle, center, flip);
}