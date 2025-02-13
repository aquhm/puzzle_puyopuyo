// GameApp.cpp
#include "GameApp.h"
#include "manager/IManager.h"
#include "manager/Managers.h"
#include "../utils/Timer.h"
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <stdexcept>
#include <format>

GameApp& GameApp::GetInstance() 
{
    static GameApp instance;
    return instance;
}

GameApp::GameApp() 
{
   
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

        // 타이머 초기화
        timer_ = std::make_unique<Timer>();
        timer_->Start();

        is_running_ = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "초기화 실패: %s", e.what());
        return false;
    }
}

bool GameApp::InitializeSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        throw std::runtime_error(std::format("SDL 초기화 실패: {}", SDL_GetError()));
    }

    SDL_SetAppMetadata("PuyoPuyo", "1.0", "com.weight.puyopuyo");

    SDL_Window* window= nullptr;
    SDL_Renderer* renderer= nullptr;

    // 렌더러 생성
    if (!SDL_CreateWindowAndRenderer(
        "PuyoPuyo",
        GAME_WIDTH,
        GAME_HEIGHT,
        SDL_WINDOW_RESIZABLE,
        &window,
        &renderer))
    {
        throw std::runtime_error(std::format("윈도우와 렌더러 생성 실패: {}", SDL_GetError()));
    }

    window_.reset(window);
    renderer_.reset(renderer);

    if (window_ == nullptr)
    {
        throw std::runtime_error(std::format("윈도우 생성 실패: {}", SDL_GetError()));
    }

    if (renderer_ == nullptr)
    {
        throw std::runtime_error(std::format("렌더러 생성 실패: {}", SDL_GetError()));
    }
}

void GameApp::MainLoop() 
{
    while (is_running_) 
    {
        elapsed_time_ = timer_->GetElapsedTime();
        accumulated_time_ += elapsed_time_;

        HandleEvents();
        Update();
        Render();
    }
}

void GameApp::HandleEvents() 
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        switch (event.type) 
        {
        case SDL_EVENT_QUIT:
            is_running_ = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key== SDLK_F11) 
            {
                SetFullscreen(!is_fullscreen_);
            }
            break;
        }

        managers_->HandleEvents(event);

        // 모든 매니저에게 이벤트 전달
        for (const auto& [name, manager] : managers_) 
        {
            if (auto* eventManager = dynamic_cast<IEventHandler*>(manager.get())) 
            {
                eventManager->HandleEvent(event);
            }
        }
    }
}

void GameApp::Update() 
{
    managers_->Update(elapsed_time_);
}

void GameApp::Render() 
{
    managers_->RenderAll(renderer_.get());
}

bool GameApp::SetFullscreen(bool enable) 
{
    
    if (is_fullscreen_ == enable) 
    {
        return true;
    }

    if (SDL_SetWindowFullscreen(window_.get(), enable) == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "전체화면 설정 실패: %s", SDL_GetError());
        return false;
    } 

    is_fullscreen_ = enable;
    return true;
}

void GameApp::Release() 
{
    // 모든 매니저 해제
    if (managers_) 
    {
        managers_->Release();
    }

    timer_.reset();
    renderer_.reset();
    window_.reset();
}