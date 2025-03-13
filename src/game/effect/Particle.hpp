#pragma once
/*
*
* 설명: 이펙트 전용 파티클 class
*
*/
#include "../RenderableObject.hpp"
#include <cstdint>

class Particle : public RenderableObject 
{
public:
    Particle() = default;
    virtual ~Particle() = default;

    Particle(const Particle&) = delete;
    Particle& operator=(const Particle&) = delete;
    Particle(Particle&&) noexcept = default;
    Particle& operator=(Particle&&) noexcept = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Release() = 0;

    [[nodiscard]] bool IsAlive() const { return is_alive_; }
    void SetAlive(bool alive) { is_alive_ = alive; }

    [[nodiscard]] float GetLifetime() const { return lifetime_; }
    void SetLifetime(float lifetime) { lifetime_ = lifetime; }

    [[nodiscard]] float GetAngle() const { return angle_; }
    void SetAngle(float angle) { angle_ = angle; }

protected:
    float size_{ 0.0f };
    float lifetime_{ 0.0f };
    float angle_{ 0.0f };
    bool is_alive_{ false };
};