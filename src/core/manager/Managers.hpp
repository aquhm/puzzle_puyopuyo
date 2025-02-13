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
    // �̱����� �ƴ� �Ϲ� Ŭ������ ���� (GameApp���� �����ֱ� ����)
    Managers() = default;
    ~Managers() = default;

    // ����/�̵� ���� ����
    Managers(const Managers&) = delete;
    Managers& operator=(const Managers&) = delete;
    Managers(Managers&&) = delete;
    Managers& operator=(Managers&&) = delete;

    // Manager ���� �޼����
    bool CreateManagers();

    template<typename T>
    [[nodiscard]] T* GetManager(std::string_view name) const;   

    // Manager ����������Ŭ ����
    bool Initialize();
    void Update(float deltaTime);
    void Release();
    void RenderAll(SDL_Renderer* renderer);
    void HandleEvents(const SDL_Event& event);
    

private:
    
    // Manager �����̳�
    std::unordered_map<std::string, std::unique_ptr<IManager>> managers_;
    std::vector<IRenderable*> renderables_;

    // Manager ���� ���� �޼���
    template<typename T>
    bool createManager() 
    {
        auto manager = std::make_unique<T>();
        auto name = manager->GetName();
        managers_[std::string(name)] = std::move(manager);
        return true;
    }    
};