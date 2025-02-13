#pragma once
#include <string>
#include <string_view>

class IResource 
{
public:
    virtual ~IResource() = default;

    [[nodiscard]] virtual bool Load(const std::string& path) = 0;
    virtual void Unload() = 0;
    [[nodiscard]] virtual bool IsLoaded() const = 0;
    [[nodiscard]] virtual std::string_view GetResourcePath() const = 0;

protected:
    IResource() = default;
};