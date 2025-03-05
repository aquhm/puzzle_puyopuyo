#include <format>
#include <algorithm>
#include <stdexcept>
#include <SDL3/SDL_render.h>

#include "../IRenderable.hpp"
#include "../IEventHandler.hpp"
#include "Managers.hpp"
#include "StateManager.hpp"
#include "ResourceManager.hpp"
#include "FontManager.hpp"
#include "MapManager.hpp"
#include "ParticleManager.hpp"
#include "PlayerManager.hpp"
#include "../../utils/Logger.hpp"


bool Managers::CreateManagers() 
{
    try 
    {
        // 각 매니저 생성        
        createManager<ResourceManager>();
        createManager<FontManager>();
        createManager<MapManager>();        
        createManager<PlayerManager>();
        createManager<StateManager>();
        createManager<ParticleManager>();


        // 렌더러블 매니저 목록 한 번만 생성
        renderables_.clear();

        for (const auto& [name, manager] : managers_) 
        {
            if (auto* renderable = dynamic_cast<IRenderable*>(manager.get())) 
            {
                renderables_.push_back(renderable);
            }
        }

        std::sort(renderables_.begin(), renderables_.end(),
            [](IRenderable* a, IRenderable* b) 
            {
                return a->GetRenderPriority() < b->GetRenderPriority();
            });

        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "매니저 생성 실패: %s", e.what());
        return false;
    }
}

bool Managers::Initialize()
{
    try
    {
        for (const auto& [name, manager] : managers_)
        {
            if (!manager->Initialize())
            {
                throw std::runtime_error(std::format("매니저 초기화 실패: {}", name));
            }
        }
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "매니저 초기화 실패: %s", e.what());
        return false;
    }
}



void Managers::Update(float dateTime) 
{
    for (const auto& [name, manager] : managers_) 
    {
        manager->Update(dateTime);
    }
}

void Managers::Release() 
{
    for (const auto& [name, manager] : managers_) 
    {
        manager->Release();
    }

    managers_.clear();
}

void Managers::RenderAll(SDL_Renderer* renderer) 
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 캐시된 렌더러블 목록 사용
    for (auto renderable : renderables_) 
    {
        renderable->Render();
    }

    SDL_RenderPresent(renderer);
}

void Managers::HandleEvents(const SDL_Event& event)
{    
   for (const auto& [name, manager] : managers_)
   {
       if (auto eventHandler = dynamic_cast<IEventHandler*>(manager.get()))
       {
           eventHandler->HandleEvent(event);
       }
   }
}