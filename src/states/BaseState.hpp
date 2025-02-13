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

    // ���� ���� �Լ����� ��Ȯ�ϰ� ����
    virtual bool Init() = 0;
    virtual void Enter() = 0;
    virtual void Leave() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;    
    virtual void Release() = 0;
    /*virtual void HandleEvent(const SDL_Event& event) = 0;
    virtual void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) = 0;*/

    // ���� �̸� ��ȯ (����� �뵵)
    [[nodiscard]] virtual std::string_view getStateName() const = 0;

    // ���� Ȯ���� ���� ��ƿ��Ƽ �Լ�
    [[nodiscard]] bool isInitialized() const { return initialized; }
    

protected:
    bool initialized{ false };

    // ���� ��ƿ��Ƽ �Լ���
    /*void checkSDLError(const std::string& operation);
    bool loadTexture(const std::string& path, SDL_Texture*& texture);*/
};