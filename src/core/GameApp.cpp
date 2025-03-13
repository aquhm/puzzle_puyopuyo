#include "GameApp.hpp"
#include "common/constants/Constants.hpp"

#include "manager/IManager.hpp"
#include "manager/Managers.hpp"
#include "manager/FontManager.hpp"
#include "manager/MapManager.hpp"
#include "manager/ParticleManager.hpp"
#include "manager/PlayerManager.hpp"
#include "manager/ResourceManager.hpp"
#include "manager/StateManager.hpp"

#include "WindowsMessageHandler.hpp"

#include "../utils/Timer.hpp"

//#include <SDL3/SDL_main.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_system.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_properties.h >

#include <stdexcept>
#include <format>
#include "../network/NetworkController.hpp"
#include "../utils/Logger.hpp"

GameApp& GameApp::GetInstance() 
{
    static GameApp instance;
    return instance;
}

GameApp::~GameApp() 
{
    Release();
    SDL_Quit();
}

bool GameApp::Initialize() 
{
    try 
    {
        InitializeSDL();

        InitializeManagers();

        windowMessageHandler_ = std::make_unique<WindowsMessageHandler>();
        windowMessageHandler_->SetupWindowsMessageHook();

        // 타이머 초기화
        timer_ = std::make_unique<Timer>();
        timer_->Start();

        is_running_ = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "초기화 실패: %s", e.what());
        return false;
    }
}

bool GameApp::InitializeSDL()
{
    SDL_SetHint(SDL_HINT_APP_NAME, "PuyoPuyo");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    
    SDL_SetAppMetadata("PuyoPuyo", "1.0", "com.weight.puyopuyo");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false)
    {
        throw std::runtime_error(std::format("SDL 초기화 실패: {}", SDL_GetError()));
    }   

    SDL_Window* window= nullptr;
    SDL_Renderer* renderer= nullptr;

    // 렌더러 생성
    if (!SDL_CreateWindowAndRenderer(
        "PuyoPuyo",
        window_width_,
        window_height_,
        SDL_WINDOW_ALWAYS_ON_TOP,
        &window,
        &renderer))
    {
        throw std::runtime_error(std::format("윈도우와 렌더러 생성 실패: {}", SDL_GetError()));
        return false;
    }

    window_.reset(window);
    renderer_.reset(renderer);

    if (window_ == nullptr)
    {
        throw std::runtime_error(std::format("윈도우 생성 실패: {}", SDL_GetError()));
        return false;
    }

    if (renderer_ == nullptr)
    {
        throw std::runtime_error(std::format("렌더러 생성 실패: {}", SDL_GetError()));
        return false;
    }

    auto props = SDL_GetWindowProperties(window_.get());

    if (SDL_GetPropertyType(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER) == SDL_PropertyType::SDL_PROPERTY_TYPE_POINTER)
    {
        hwnd_ = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    }

    Uint32 myEventType = SDL_RegisterEvents(1);
    if (myEventType != -1)
    {
        SDL_Event event{};
        event.type = SDL_USEREVENT_SOCK;
        SDL_PushEvent(&event);
    }

    return true;
}

void GameApp::InitializeManagers()
{
    managers_ = std::make_unique<Managers>();

    managers_->CreateManagers();

    // 캐시 포인터 초기화
    resource_manager_ = managers_->GetManager<ResourceManager>("ResourceManager");    
    font_manager_ = managers_->GetManager<FontManager>("FontManager");
    map_manager_ = managers_->GetManager<MapManager>("MapManager");
    player_manager_ = managers_->GetManager<PlayerManager>("PlayerManager");
    particle_manager_ = managers_->GetManager<ParticleManager>("ParticleManager");
    state_manager_ = managers_->GetManager<StateManager>("StateManager");

    // nullptr 체크
    if (!resource_manager_ || !state_manager_ || !font_manager_ ||
        !map_manager_ || !player_manager_ || !particle_manager_)
    {
        throw std::runtime_error("Failed to initialize one or more managers");
    }

    if (!managers_->Initialize())
    {
        throw std::runtime_error(std::format("Failed to initialize manager"));
    }
}

void GameApp::MainLoop() 
{
    //while (is_running_) 
    {
        elapsed_time_ = timer_->GetElapsedTime();
        accumulated_time_ += elapsed_time_;

        //HandleEvents();
        Update();
        Render();
    }
}

void GameApp::HandleEvents(const SDL_Event& event)
{
    /*SDL_Event event;
    while (SDL_PollEvent(&event)) */
    {
        switch (event.type) 
        {
        case SDL_EVENT_QUIT:
            is_running_ = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key== SDLK_F11) 
            {
                SetFullscreen(!is_full_screen_);
            }
            break;
        case SDL_EVENT_USER:
            if (event.user.code == Constants::Network::NETWORK_EVENT_CODE) 
            {
                SOCKET socket_handle = reinterpret_cast<SOCKET>(event.user.data1);
                LONG network_events = static_cast<LONG>(reinterpret_cast<uintptr_t>(event.user.data2));

                NETWORK.ProcessRecv(static_cast<WPARAM>(socket_handle), static_cast<LPARAM>(network_events)
                );
            }
            break;
        }

        managers_->HandleEvents(event);
    }
}

void GameApp::Update() 
{
    if (NETWORK.IsRunning())
    {
        NETWORK.Update();
    }

    managers_->Update(elapsed_time_);
}

void GameApp::Render() 
{
    managers_->RenderAll(renderer_.get());
}

bool GameApp::SetFullscreen(bool enable) 
{
    
    if (is_full_screen_ == enable) 
    {
        return true;
    }

    if (SDL_SetWindowFullscreen(window_.get(), enable) == false)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_VIDEO, "전체화면 설정 실패: %s", SDL_GetError());
        return false;
    } 

    is_full_screen_ = enable;
    return true;
}

void GameApp::Release() 
{ 

    // 모든 매니저 해제
    if (managers_)
    {
        managers_->Release();
    }

    resource_manager_ = nullptr;
    state_manager_ = nullptr;
    font_manager_ = nullptr;
    map_manager_ = nullptr;
    player_manager_ = nullptr;
    particle_manager_ = nullptr;

    timer_.reset();
    renderer_.reset();
    window_.reset();
}