// IManager.h
#pragma once
#include <string_view>

class IManager 
{
public:
    virtual ~IManager() = default;
    [[nodiscard]] virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Release() = 0;
    [[nodiscard]] virtual std::string_view GetName() const = 0;
};