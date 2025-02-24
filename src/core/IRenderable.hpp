#pragma once

/**
 *
 * 설명: 오브젝트 render 추상화 Interface
 * 1. 그려지는 대상은 이 클래스는 이 Interface를 상속 구현
 * 
 */


class IRenderable 
{

public:
    virtual ~IRenderable() = default;
    virtual void Render() = 0;
    [[nodiscard]] virtual int GetRenderPriority() const { return 0; }
};