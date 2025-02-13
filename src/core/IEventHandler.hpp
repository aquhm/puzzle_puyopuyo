// IEventHandler.h
#pragma once
#include <SDL3/SDL.h>


class IEventHandler 
{

public:
    virtual ~IEventHandler() = default;
    virtual void HandleEvent(const SDL_Event& event) = 0;
};