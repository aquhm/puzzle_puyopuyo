// Managers.h
#pragma once

#include "IManager.hpp"
#include <memory>
#include <unordered_map>
#include <string_view>

class IRenderable;
class IEventHandler;
class SDL_Renderer;

class Managers 
{

public:
    // 싱글톤이 아닌 일반 클래스로 설계 (GameApp에서 생명주기 관리)
    Managers() = default;
    ~Managers() = default;

    // 복사/이동 연산 방지
    Managers(const Managers&) = delete;
    Managers& operator=(const Managers&) = delete;
    Managers(Managers&&) = delete;
    Managers& operator=(Managers&&) = delete;

    // Manager 생성 메서드들
    bool CreateManagers();

    template<typename T>
    [[nodiscard]] T* GetManager(std::string_view name) const;   

    // Manager 라이프사이클 관리
    bool Initialize();
    void Update(float deltaTime);
    void Release();
    void RenderAll(SDL_Renderer* renderer);
    void HandleEvents(const SDL_Event& event);
    

private:
    
    // Manager 컨테이너
    std::unordered_map<std::string, std::unique_ptr<IManager>> managers_;
    std::vector<IRenderable*> renderables_;

    // Manager 생성 헬퍼 메서드
    template<typename T>
    bool createManager() 
    {
        auto manager = std::make_unique<T>();
        auto name = manager->GetName();
        managers_[std::string(name)] = std::move(manager);
        return true;
    }    
};