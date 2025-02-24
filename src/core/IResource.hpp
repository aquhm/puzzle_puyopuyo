#pragma once
/*
 *
 * 설명: 오브젝트 render 추상화 Interface
 * 1. 그려지는 대상은 이 클래스는 이 Interface를 상속 구현
 *
 */


#include <string>
#include <string_view>

class IResource 
{
public:
    IResource() = default;
    virtual ~IResource() = default;

    virtual void Unload() = 0;
    [[nodiscard]] virtual bool Load(const std::string& path) = 0;    
    [[nodiscard]] virtual bool IsLoaded() const = 0;
    [[nodiscard]] virtual std::string_view GetResourcePath() const = 0;
    
};