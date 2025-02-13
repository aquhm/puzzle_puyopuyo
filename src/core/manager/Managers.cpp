// Managers.cpp

#include <format>
#include <algorithm>
#include <stdexcept>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_log.h>

#include "Managers.hpp"
#include "../IRenderable.hpp"
#include "StateManager.hpp"

//#include "StateManager.h"
//#include "ResourceManager.h"
//#include "FontManager.h"
//#include "ParticleManager.h"
//#include "NetworkManager.h"



bool Managers::CreateManagers() 
{
    try 
    {
        // �� �Ŵ��� ����
        createManager<StateManager>();
        /*CreateManager<ResourceManager>();
        CreateManager<FontManager>();
        CreateManager<ParticleManager>();
        CreateManager<NetworkManager>();*/

        // �������� �Ŵ��� ��� �� ���� ����
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "�Ŵ��� ���� ����: %s", e.what());
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
                throw std::runtime_error(std::format("�Ŵ��� �ʱ�ȭ ����: {}", name));
            }
        }
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "�Ŵ��� �ʱ�ȭ ����: %s", e.what());
        return false;
    }
}

template<typename T>
[[nodiscard]] T* Managers::GetManager(std::string_view name) const
{
    if (auto it = managers_.find(std::string(name)); it != managers_.end())
    {
        return dynamic_cast<T*>(it->second.get());
    }

    return nullptr;
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

    // ĳ�õ� �������� ��� ���
    for (auto* renderable : renderables_) 
    {
        renderable->Render();
    }

    SDL_RenderPresent(renderer);
}

void Managers::HandleEvents(const SDL_Event& event)
{
    for (const auto& [name, manager] : managers_)
    {
        if (auto* eventHandler = dynamic_cast<IEventHandler*>(manager.get()))
        {
            eventHandler->HandleEvent(event);
        }
    }
}