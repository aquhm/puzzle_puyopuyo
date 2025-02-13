#pragma once

#include "ResourceCache.hpp"
#include <SDL.h>

class GameResources {
public:
    struct TextureInfo {
        SDL_Texture* texture;
        int width;
        int height;
    };

    static GameResources& getInstance() {
        static GameResources instance;
        return instance;
    }

    bool preloadGameTextures();
    bool preloadCharacterTextures(int characterId);
    void releaseUnusedResources();

private:
    ResourceCache<TextureInfo> textureCache;
    ResourceCache<SDL_Mix_Chunk> soundCache;

    GameResources() = default;
    ~GameResources() = default;
};