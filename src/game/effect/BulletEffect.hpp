#pragma once


#include <memory>
#include <array>
#include <SDL3/SDL.h>
#include <array>

#include "../RenderableObject.hpp"

class ImgTexture;
enum class BlockType;

namespace effects {

    class BulletEffect : public RenderableObject {
    public:
        static constexpr float BULLET_AMPLITUDE = 100.0f;    // 발사체 진폭 크기
        static constexpr float BULLET_FORCE = 1500.0f;      // 발사체 기본 힘
        static constexpr float SHOCK_EXPAND_TIME = 0.3f;
        static constexpr float SHOCK_EXPAND_DELTA_SIZE = 100.0f;
        static constexpr float SHOCK_EXPAND_POS_VELOCITY = (SHOCK_EXPAND_DELTA_SIZE * 0.5f) / SHOCK_EXPAND_TIME;
        static constexpr float PI = 3.14159265358979323846f;

        enum class State {
            Moving,     // 이동 중
            Shocking,   // 충격파 효과 중
            Out        // 소멸
        };

        BulletEffect();
        ~BulletEffect() override = default;

        // Delete copy/move operations
        BulletEffect(const BulletEffect&) = delete;
        BulletEffect& operator=(const BulletEffect&) = delete;
        BulletEffect(BulletEffect&&) = delete;
        BulletEffect& operator=(BulletEffect&&) = delete;

        // Core functionality
        void Update(float deltaTime) override;
        void Render() override;
        void Release() override;
        [[nodiscard]] bool Initialize(const SDL_FPoint& start_pos,
            const SDL_FPoint& target_pos,
            BlockType type);

        // State queries
        [[nodiscard]] bool IsAlive() const { return is_alive_; }
        [[nodiscard]] bool IsAttacking() const { return is_attacking_; }
        [[nodiscard]] State GetState() const { return current_state_; }
        [[nodiscard]] BlockType GetBlockType() const { return block_type_; }
        [[nodiscard]] int16_t GetBlockCount() const { return block_count_; }

        // State setters
        void SetAttacking(bool attacking) { is_attacking_ = attacking; }
        void SetBlockCount(int16_t count) { block_count_ = count; }

    private:
        struct TextureRects {
            SDL_Rect head{};     // 총알 머리 부분
            SDL_Rect tail{};     // 총알 꼬리 부분
            std::array<SDL_Rect, 2> shock{};  // 충격파 효과
        };

        // Helper functions
        [[nodiscard]] static float ToRadians(float degrees) {
            return degrees * PI / 180.0f;
        }
        [[nodiscard]] static float ToDegrees(float radians) {
            return radians * 180.0f / PI;
        }

        // Resources
        std::shared_ptr<ImgTexture> texture_;
        TextureRects rects_;

        // Movement
        SDL_FPoint start_pos_{};
        SDL_FPoint target_pos_{};
        SDL_FPoint velocity_{};
        SDL_FPoint direction_{};
        SDL_FPoint previous_pos_{};
        SDL_FPoint shock_pos_{};
        SDL_Point rotation_center_{};

        // State
        State current_state_{ State::Out };
        BlockType block_type_{};
        bool is_alive_{ false };
        bool is_attacking_{ false };
        int16_t block_count_{ 0 };

        // Animation
        float accumulated_time_{ 0.0f };
        float accumulated_angle_{ 0.0f };
        float tail_angle_{ 0.0f };
        float scale_{ 100.0f };
        float scale_velocity_{ 0.0f };
        uint8_t alpha_{ 255 };

        // Internal methods
        void UpdateMovement(float delta_time);
        void UpdateShockwave(float delta_time);
        void SetupTextureRects();
        void RenderMoving() const;
        void RenderShockwave() const;
    };