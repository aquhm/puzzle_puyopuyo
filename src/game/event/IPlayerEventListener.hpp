#pragma once
#include "PlayerEvent.hpp"
#include <memory>

class IPlayerEventListener
{
public:
    virtual ~IPlayerEventListener() = default;
    virtual void OnPlayerEvent(const std::shared_ptr<BasePlayerEvent>& event) = 0;
};