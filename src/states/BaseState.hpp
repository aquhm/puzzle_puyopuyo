#pragma once
/*
 *
 * 설명: 게임 기본 상태 추상 클래스
 *
 */
#include <memory>
#include <string_view>
#include <span>
#include <SDL3/SDL_events.h>

class BaseState 
{
public:
    virtual ~BaseState() = default;

    virtual bool Init() = 0;
    virtual void Enter() = 0;
    virtual void Leave() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;    
    virtual void Release() = 0;
    virtual void HandleEvent(const SDL_Event& event) = 0;
    virtual void HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) = 0;

    [[nodiscard]] virtual std::string_view GetStateName() const = 0;

    [[nodiscard]] bool isInitialized() const { return initialized; }
    

protected:
    bool initialized{ false };

};