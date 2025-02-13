#pragma once

class IRenderable 
{

public:
    virtual ~IRenderable() = default;
    virtual void Render() = 0;
    [[nodiscard]] virtual int GetRenderPriority() const { return 0; }
};