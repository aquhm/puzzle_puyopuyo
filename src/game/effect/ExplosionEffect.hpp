#pragma once
/*
*
* 설명: 블록 제거시 폭발하는 연출의 파티클 클래스
*
*/

#include "../RenderableObject.hpp"
#include "Particle.hpp"
#include "ParticleContainer.hpp"

#include <vector>
#include <memory>
#include <random>


enum class BlockType;
struct SDL_FPoint;
struct SDL_FRect;

class ExplosionParticle final : public Particle 
{
public:
    ExplosionParticle();
    ~ExplosionParticle() override = default;

    ExplosionParticle(const ExplosionParticle&) = delete;
    ExplosionParticle& operator=(const ExplosionParticle&) = delete;
    ExplosionParticle(ExplosionParticle&&) noexcept = default;
    ExplosionParticle& operator=(ExplosionParticle&&) noexcept = default;

    void Update(float deltaTime) override {}
    void Render() override {}
    void Release() override {}

private:
    friend class ExplosionContainer;

    BlockType type_;
    SDL_FPoint direction_{};
    SDL_FPoint velocity_{};
    float force_{ 0.0f };
};

class ExplosionContainer final : public ParticleContainer {
public:
    ExplosionContainer();
    ~ExplosionContainer() override = default;

    ExplosionContainer(const ExplosionContainer&) = delete;
    ExplosionContainer& operator=(const ExplosionContainer&) = delete;
    ExplosionContainer(ExplosionContainer&&) noexcept = default;
    ExplosionContainer& operator=(ExplosionContainer&&) noexcept = default;

    void Update(float deltaTime) override;
    void Render() override;
    bool InitializeParticles();

    void SetBlockType(BlockType type);

private:

    void UpdateParticlePhysics(ExplosionParticle& particle, float deltaTime);

private:

    BlockType type_;
};