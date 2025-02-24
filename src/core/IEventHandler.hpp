#pragma once
#include <SDL3/SDL.h>

/**
 *
 * 설명: 오브젝트 Event 추상화 Interface
 * 1. SDL_Event 정보를 수신 처리가 필요한 클래스는 이 Interface를 상속 구현
 *
 */


class IEventHandler 
{

public:
    virtual ~IEventHandler() = default;
    virtual void HandleEvent(const SDL_Event& event) = 0;
};