#include "ImageTexture.hpp"
#include "../core/manager/ResourceManager.hpp"
#include "../core/GameApp.hpp"

#include <SDL3/SDL_image.h>
#include <stdexcept>
#include <format>

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

std::shared_ptr<ImageTexture> ImageTexture::Create(const std::string& path)
{
    return GAME_APP.GetResourceManager().GetResource<ImageTexture>(path);
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

    try
    { 
        if (auto formatDetail = SDL_GetPixelFormatDetails(loadedSurface->format); formatDetail != nullptr)
        {
            auto colorKey = SDL_MapRGB(formatDetail, NULL, 0, 0, 0);
            SDL_SetSurfaceColorKey(loadedSurface, true, colorKey);
        }

        texture_ = SDL_CreateTextureFromSurface(GAME_APP.GetRenderer(), loadedSurface);
        if (texture_ == nullptr)
        {
            SDL_DestroySurface(loadedSurface);
            throw std::runtime_error(std::format("Unable to create texture: {}", SDL_GetError()));
        }

        width_ = static_cast<float>(loadedSurface->w);
        height_ = static_cast<float>(loadedSurface->h);

        SDL_DestroySurface(loadedSurface);
    }
    catch (const std::exception&)
    {
        SDL_DestroySurface(loadedSurface);
    }
    
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

void ImageTexture::Render(float x, float y, const SDL_FRect* sourceRect, double angle, const SDL_FPoint* center, SDL_FlipMode flip) const
{
    if (texture_ == nullptr)
    {
        return;
    }

    auto renderer = GAME_APP.GetRenderer();
    if (renderer == nullptr)
    {
        return;
    }

    SDL_FRect destRect{ x, y,
        sourceRect ? sourceRect->w : width_,
        sourceRect ? sourceRect->h : height_ };

    SDL_RenderTextureRotated(renderer, texture_, sourceRect, &destRect, angle, center, flip);
}

void ImageTexture::RenderScaled(const SDL_FRect* sourceRect, const SDL_FRect* destRect, double angle, const SDL_FPoint* center, SDL_FlipMode flip) const 
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