#pragma once

#include "../core/IResource.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <memory>
//#include "../core/GameApp.hpp"
//#include "../core/manager/ResourceManager.hpp"

class ResourceManager;
class GameApp;

class ImageTexture : public IResource 
{
public:

    friend class ResourceManager;

    ImageTexture() = default;
    ~ImageTexture() override;

    ImageTexture(const ImageTexture&) = delete;
    ImageTexture& operator=(const ImageTexture&) = delete;
    ImageTexture(ImageTexture&&) noexcept;
    ImageTexture& operator=(ImageTexture&&) noexcept;

    static std::shared_ptr<ImageTexture> Create(const std::string& path);
    
    [[nodiscard]] bool IsLoaded() const override { return texture_ != nullptr; }
    [[nodiscard]] std::string_view GetResourcePath() const override { return path_; }

    [[nodiscard]] float GetWidth() const { return width_; }
    [[nodiscard]] float GetHeight() const { return height_; }
    [[nodiscard]] SDL_Texture* GetSDLTexture() const { return texture_; }

    void Unload() override;
    void SetColor(uint8_t red, uint8_t green, uint8_t blue);
    void SetBlendMode(SDL_BlendMode blending);
    void SetAlpha(uint8_t alpha);

    void Render(float x, float y, const SDL_FRect* sourceRect = nullptr,
        double angle = 0.0, const SDL_FPoint* center = nullptr,
        SDL_FlipMode flip = SDL_FLIP_NONE) const;

    void RenderScaled(const SDL_FRect* sourceRect, const SDL_FRect* destRect,
        double angle = 0.0, const SDL_FPoint* center = nullptr,
        SDL_FlipMode flip = SDL_FLIP_NONE) const;

protected:
    [[nodiscard]] bool Load(const std::string& path) override;
    void ReleaseTexture();

protected:

    SDL_Texture* texture_{ nullptr };
    std::string path_;
    float width_{ 0 };
    float height_{ 0 };    
};