// GameApp.h
#pragma once

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <format>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <atomic>
#include "manager/ResourceManager.hpp"

// Forward declarations
class IManager;
class Timer;
class Managers;

class GameApp {
public:
    static constexpr int GAME_WIDTH = 640;
    static constexpr int GAME_HEIGHT = 448;
    static constexpr int GAME_FPS = 60;

    // Singleton accessor
    static GameApp& GetInstance();

    // Delete copy/move operations
    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;
    GameApp(GameApp&&) = delete;
    GameApp& operator=(GameApp&&) = delete;

    // Core game functions
    bool Initialize();
    void Release();
    void MainLoop();
   

    // Getters
    [[nodiscard]] auto GetWindow() const noexcept { return window_.get(); }
    [[nodiscard]] auto GetRenderer() const noexcept { return renderer_.get(); }
    [[nodiscard]] bool IsGameRunning() const noexcept { return is_running_; }
    [[nodiscard]] int GetWindowWidth() const noexcept { return window_width_; }
    [[nodiscard]] int GetWindowHeight() const noexcept { return window_height_; }
    [[nodiscard]] bool IsFullscreen() const noexcept { return is_fullscreen_; }
    [[nodiscard]] float GetAccumulatedTime() const noexcept { return accumulated_time_; }
    [[nodiscard]] float GetElapsedTime() const noexcept { return elapsed_time_; }
    

    
    template<typename T>
    [[nodiscard]] T* GetManager(std::string_view name) const
    {
        return managers_->GetManager<T>(name);
    }

    [[nodiscard]] Managers& GetManagers() const 
    {
        if (!managers_) {
            throw std::runtime_error("Managers not initialized");
        }
        return *managers_;
    }

    [[nodiscard]] ResourceManager& GetResourceManager() const 
    {
        return *GetManager<ResourceManager>("ResourceManager");
    }

    [[nodiscard]] ResourceManager& GetResourceManager() const
    {
        return *GetManager<ResourceManager>("ResourceManager");
    }

private:
    GameApp();
    ~GameApp();

    // SDL resource management
    struct SDLDeleter {
        void operator()(SDL_Window* window) const {
            if (window) SDL_DestroyWindow(window);
        }
        void operator()(SDL_Renderer* renderer) const {
            if (renderer) SDL_DestroyRenderer(renderer);
        }
    };

    std::unique_ptr<Managers> managers_;

    std::unique_ptr<SDL_Window, SDLDeleter> window_;
    std::unique_ptr<SDL_Renderer, SDLDeleter> renderer_;
    std::unique_ptr<Timer> timer_;


    // State variables
    std::atomic<bool> is_running_{ false };
    std::atomic<bool> is_fullscreen_{ false };
    int window_width_{ GAME_WIDTH };
    int window_height_{ GAME_HEIGHT };
    float accumulated_time_{ 0.0f };
    float elapsed_time_{ 0.0f };

    // Core game loop methods
    bool InitializeSDL();
    void HandleEvents();
    void Update();
    void Render();
    bool SetFullscreen(bool enable);
};

// Global access macro
#define GAME_APP GameApp::GetInstance()