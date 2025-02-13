// states/BaseState.hpp
#pragma once

#include <memory>
#include <string_view>
//#include "core/GameConfig.hpp"
//#include "network/NetworkTypes.hpp"
#include <SDL3/SDL_events.h>

class BaseState {
public:
    virtual ~BaseState() = default;

    // 순수 가상 함수들을 명확하게 정의
    virtual bool Init() = 0;
    virtual void Enter() = 0;
    virtual void Leave() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;    
    virtual void Release() = 0;
    /*virtual void HandleEvent(const SDL_Event& event) = 0;
    virtual void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) = 0;*/

    // 상태 이름 반환 (디버깅 용도)
    [[nodiscard]] virtual std::string_view getStateName() const = 0;

    // 상태 확인을 위한 유틸리티 함수
    [[nodiscard]] bool isInitialized() const { return initialized; }
    

protected:
    bool initialized{ false };

    // 공통 유틸리티 함수들
    /*void checkSDLError(const std::string& operation);
    bool loadTexture(const std::string& path, SDL_Texture*& texture);*/
};