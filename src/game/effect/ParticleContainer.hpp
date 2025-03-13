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
    [[nodiscard]] int GetAliveCount() const { return alive_count_; }

private:
    int alive_count_{ 0 };
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
    [[nodiscard]] bool IsAlive() const { return initial_lifetime_ > accumulated_lifetime_; }
    [[nodiscard]] uint8_t GetPlayerID() const { return player_id_; }

    void SetPosition(const SDL_FPoint& pos) { position_ = pos; }
    void SetTexture(std::shared_ptr<ImageTexture> texture) { source_texture_ = texture; }
    void SetPlayerID(uint8_t id) { player_id_ = id; }

protected:

    void RemoveDeadParticles();
    void ClearParticles();

protected:
    SDL_FPoint position_{};
    float initial_lifetime_{ 0.0f };
    float accumulated_lifetime_{ 0.0f };
    size_t max_particles_{ 0 };
    float limit_radius_{ 0.0f };

    std::vector<std::unique_ptr<Particle>> particles_;
    std::shared_ptr<ImageTexture> source_texture_;
    SDL_FRect source_rect_{};
    uint8_t player_id_{ 0 };

};
