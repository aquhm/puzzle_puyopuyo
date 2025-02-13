#pragma once

#include "../core/IResource.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <memory>

class ImageTexture : public IResource 
{
public:
    ImageTexture() = default;
    ~ImageTexture() override;

    // IResource interface implementation
    [[nodiscard]] bool Load(const std::string& path) override;
    void Unload() override;
    [[nodiscard]] bool IsLoaded() const override { return texture_ != nullptr; }
    [[nodiscard]] std::string_view GetResourcePath() const override { return path_; }

    // Color manipulation
    void SetColor(uint8_t red, uint8_t green, uint8_t blue);
    void SetBlendMode(SDL_BlendMode blending);
    void SetAlpha(uint8_t alpha);

    // Rendering methods
    void Render(int x, int y, const SDL_FRect* sourceRect = nullptr,
        double angle = 0.0, const SDL_FPoint* center = nullptr,
        SDL_FlipMode flip = SDL_FLIP_NONE) const;

    void RenderScaled(const SDL_FRect* sourceRect, const SDL_FRect* destRect,
        double angle = 0.0, const SDL_FPoint* center = nullptr,
        SDL_FlipMode flip = SDL_FLIP_NONE) const;

    // Accessors
    [[nodiscard]] int GetWidth() const { return width_; }
    [[nodiscard]] int GetHeight() const { return height_; }
    [[nodiscard]] SDL_Texture* GetSDLTexture() const { return texture_; }

    // Delete copy operations
    ImageTexture(const ImageTexture&) = delete;
    ImageTexture& operator=(const ImageTexture&) = delete;

    // Allow move operations
    ImageTexture(ImageTexture&&) noexcept;
    ImageTexture& operator=(ImageTexture&&) noexcept;

private:


    SDL_Texture* texture_{ nullptr };
    std::string path_;
    int width_{ 0 };
    int height_{ 0 };

    // Helper methods
    void ReleaseTexture();
};