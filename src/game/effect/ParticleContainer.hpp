#pragma once

/*
*
* 설명: 파티클 생성후 파티클의 Life Cycle을 관리하는 Container
*
*/

#include <vector>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <SDL3/SDL_rect.h>

class Particle;
class ImageTexture;
struct SDL_FRect;
struct SDL_FPoint;


class ParticleAliveChecker
{
public:
    ParticleAliveChecker() = default;

    void operator()(const std::unique_ptr<Particle>& particle);
    [[nodiscard]] int GetAliveCount() const { return aliveCount_; }

private:
    int aliveCount_{ 0 };
};

class ParticleContainer
{
public:
    ParticleContainer() = default;
    virtual ~ParticleContainer();

    ParticleContainer(const ParticleContainer&) = delete;
    ParticleContainer& operator=(const ParticleContainer&) = delete;
    ParticleContainer(ParticleContainer&&) noexcept = default;
    ParticleContainer& operator=(ParticleContainer&&) noexcept = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual bool InitializeParticles() = 0;
    virtual void Release();

    [[nodiscard]] int GetAliveParticleCount() const;
    [[nodiscard]] bool IsAlive() const { return initialLifetime_ > accumulatedLifetime_; }
    [[nodiscard]] uint8_t GetPlayerID() const { return playerID_; }

    void SetPosition(const SDL_FPoint& pos) { position_ = pos; }
    void SetTexture(std::shared_ptr<ImageTexture> texture) { sourceTexture_ = texture; }
    void SetPlayerID(uint8_t id) { playerID_ = id; }

protected:

    void RemoveDeadParticles();
    void ClearParticles();

protected:
    SDL_FPoint position_{};
    float initialLifetime_{ 0.0f };
    float accumulatedLifetime_{ 0.0f };
    size_t maxParticles_{ 0 };
    float limitRadius_{ 0.0f };

    std::vector<std::unique_ptr<Particle>> particles_;
    std::shared_ptr<ImageTexture> sourceTexture_;
    SDL_FRect sourceRect_{};
    uint8_t playerID_{ 0 };

};
