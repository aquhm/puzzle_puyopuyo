#pragma once

#include "../../utils/Math.hpp"
#include <vector>
#include <memory>
#include <chrono>

class ImgTexture;
struct SDL_Rect;

namespace effects {

    class Particle;

    struct ParticleConfig {
        std::chrono::milliseconds initial_lifetime{ 1000 };  // Default 1 second
        float max_radius{ 100.0f };
        int max_particles{ 100 };
        SDL_Rect source_rect{};
    };

    class ParticleContainer {
    public:
        ParticleContainer() = default;
        virtual ~ParticleContainer() = default;

        // Delete copy/move operations
        ParticleContainer(const ParticleContainer&) = delete;
        ParticleContainer& operator=(const ParticleContainer&) = delete;
        ParticleContainer(ParticleContainer&&) = delete;
        ParticleContainer& operator=(ParticleContainer&&) = delete;

        // Core functionality
        virtual void Update(float deltaTime) = 0;
        virtual void Render() = 0;
        [[nodiscard]] virtual bool Initialize() = 0;
        virtual void Release();

        // State queries
        [[nodiscard]] int GetAliveParticleCount() const;
        [[nodiscard]] bool IsActive() const { return is_active_; }
        [[nodiscard]] bool IsInitialized() const { return is_initialized_; }

        // Position management
        void SetPosition(const Vector2f& position) { position_ = position; }
        void SetPosition(float x, float y) { position_ = { x, y }; }
        [[nodiscard]] const Vector2f& GetPosition() const { return position_; }

        // Configuration
        void SetConfig(const ParticleConfig& config) { config_ = config; }
        void SetTexture(std::shared_ptr<ImgTexture> texture) { texture_ = texture; }
        void SetPlayerId(uint8_t id) { player_id_ = id; }
        [[nodiscard]] uint8_t GetPlayerId() const { return player_id_; }

    protected:
        std::vector<std::unique_ptr<Particle>> particles_;
        std::shared_ptr<ImgTexture> texture_;
        ParticleConfig config_;
        Vector2f position_{ 0.0f, 0.0f };
        bool is_active_{ false };
        bool is_initialized_{ false };
        uint8_t player_id_{ 0 };

        virtual void RespawnParticle(Particle& particle) = 0;
        virtual void UpdateParticle(Particle& particle, float deltaTime) = 0;
    };

} // namespace effects