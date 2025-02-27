#pragma once
/*
 *
 * 설명: IManager 구현한 Manager 클래스 생성 및 라이프 사이클 관리
 *
 */

#include "IManager.hpp"
#include <memory>
#include <unordered_map>
#include <string_view>
#include <concepts>

class IRenderable;
class IEventHandler;
struct SDL_Renderer;

class Managers 
{

public:
    Managers() = default;
    ~Managers() = default;

    Managers(const Managers&) = delete;
    Managers& operator=(const Managers&) = delete;
    Managers(Managers&&) = delete;
    Managers& operator=(Managers&&) = delete;

    bool CreateManagers();

    template<std::derived_from<IManager> T>
    [[nodiscard]] T* GetManager(std::string_view name) const
    {
        if (auto it = managers_.find(std::string(name)); it != managers_.end())
        {
            return dynamic_cast<T*>(it->second.get());
        }

        return nullptr;
    }

    bool Initialize();
    void Update(float deltaTime);
    void Release();
    void RenderAll(SDL_Renderer* renderer);
    void HandleEvents(const SDL_Event& event);
    

private:
    template<std::derived_from<IManager> T>
    bool createManager()
    {
        auto manager = std::make_unique<T>();
        auto [it, inserted] = managers_.emplace(manager->GetName(), std::move(manager));
        return inserted;
    }

private:
    
    std::unordered_map<std::string, std::unique_ptr<IManager>> managers_;
    std::vector<IRenderable*> renderables_;   
};