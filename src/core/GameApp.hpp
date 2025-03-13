#pragma once
/**
 *
 * 설명: 프로그램 라이프 사이클 관리 및 Manger Getter
 *
 */

#include <SDL3/SDL.h>

#include <format>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <atomic>
#include <concepts>
#include <wtypes.h>

#include "./common/constants/Constants.hpp"
#include "manager/IManager.hpp"
#include "manager/Managers.hpp"


class IManager;
class Timer;
class Managers;
class ResourceManager;
class StateManager;
class FontManager;
class MapManager;
class PlayerManager;
class ParticleManager;
class WindowsMessageHandler;


class GameApp 
{
public:

    static GameApp& GetInstance();

    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;
    GameApp(GameApp&&) = delete;
    GameApp& operator=(GameApp&&) = delete;

    bool Initialize();
    void Release();
    void MainLoop();
    void HandleEvents(const SDL_Event& event);   

    [[nodiscard]] auto GetWindow() const noexcept { return window_.get(); }
    [[nodiscard]] auto GetRenderer() const noexcept { return renderer_.get(); }
    [[nodiscard]] bool IsGameRunning() const noexcept { return is_running_; }
    [[nodiscard]] int GetWindowWidth() const noexcept { return window_width_; }
    [[nodiscard]] int GetWindowHeight() const noexcept { return window_height_; }
    [[nodiscard]] bool IsFullscreen() const noexcept { return is_full_screen_; }
    [[nodiscard]] float GetAccumulatedTime() const noexcept { return accumulated_time_; }
    [[nodiscard]] float GetElapsedTime() const noexcept { return elapsed_time_; }

    void SetGameRunning(bool running){ is_running_ = running; }
    
    template<std::derived_from<IManager> T>
    [[nodiscard]] T* GetManager(std::string_view name) const
    {
        return managers_->GetManager<T>(name);
    }

    [[nodiscard]] Managers& GetManagers() const 
    {
        if (!managers_) 
        {
            throw std::runtime_error("Managers not initialized");
        }
        return *managers_;
    }

    // Manager Getter
    [[nodiscard]] ResourceManager& GetResourceManager() const 
    {
        if (!resource_manager_) throw std::runtime_error("ResourceManager not initialized");
        return *resource_manager_;
    }

    [[nodiscard]] StateManager& GetStateManager() const 
    {
        if (!state_manager_) throw std::runtime_error("StateManager not initialized");
        return *state_manager_;
    }

    [[nodiscard]] FontManager& GetFontManager() const 
    {
        if (!font_manager_) throw std::runtime_error("FontManager not initialized");
        return *font_manager_;
    }

    [[nodiscard]] MapManager& GetMapManager() const 
    {
        if (!map_manager_) throw std::runtime_error("MapManager not initialized");
        return *map_manager_;
    }

    [[nodiscard]] PlayerManager& GetPlayerManager() const 
    {
        if (!player_manager_) throw std::runtime_error("PlayerManager not initialized");
        return *player_manager_;
    }

    [[nodiscard]] ParticleManager& GetParticleManager() const 
    {
        if (!particle_manager_) throw std::runtime_error("ParticleManager not initialized");
        return *particle_manager_;
    }
    [[nodiscard]] const HWND& GetWindowHandle() const { return hwnd_; }


private:
    GameApp() = default;
    ~GameApp();

    bool InitializeSDL();
    void InitializeManagers();    
    void Update();
    void Render();
    bool SetFullscreen(bool enable);

    struct SDLDeleter {
        void operator()(SDL_Window* window) const 
        {
            if (window) SDL_DestroyWindow(window);
        }

        void operator()(SDL_Renderer* renderer) const 
        {
            if (renderer) SDL_DestroyRenderer(renderer);
        }
    };

    std::unique_ptr<Managers> managers_;
    std::unique_ptr<SDL_Window, SDLDeleter> window_;
    std::unique_ptr<SDL_Renderer, SDLDeleter> renderer_;
    std::unique_ptr<Timer> timer_;
    std::unique_ptr<WindowsMessageHandler> windowMessageHandler_;

    std::atomic<bool> is_running_{ false };
    std::atomic<bool> is_full_screen_{ false };
    int window_width_{ Constants::Window::DEFAULT_WIDTH };
    int window_height_{ Constants::Window::DEFAULT_HEIGHT };
    float accumulated_time_{ 0.0f };
    float elapsed_time_{ 0.0f };    

    HWND hwnd_;

    ResourceManager* resource_manager_{ nullptr };
    StateManager* state_manager_{ nullptr };
    FontManager* font_manager_{ nullptr };
    MapManager* map_manager_{ nullptr };
    PlayerManager* player_manager_{ nullptr };
    ParticleManager* particle_manager_{ nullptr };
};

#define GAME_APP GameApp::GetInstance()