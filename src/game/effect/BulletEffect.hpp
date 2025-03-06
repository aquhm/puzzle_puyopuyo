#pragma once
/*
*
* 설명: 블록 콤보의 의해 상대방에게 공격 연출하는 발사체 연출 파티클 Class
*
*/

#include <memory>
#include <array>
#include <SDL3/SDL.h>

#include "../RenderableObject.hpp"

class ImageTexture;
enum class BlockType;
struct SDL_FPoint;


class BulletEffect : public RenderableObject 
{
public:
    

    enum class State 
    {
        Moving,     // 이동 중
        Shocking,   // 충격파 효과 중
        Out         // 소멸
    };

    BulletEffect();
    ~BulletEffect() override = default;
    BulletEffect(const BulletEffect&) = delete;
    BulletEffect& operator=(const BulletEffect&) = delete;
    BulletEffect(BulletEffect&&) = delete;
    BulletEffect& operator=(BulletEffect&&) = delete;

    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;
    [[nodiscard]] bool Initialize(const SDL_FPoint& start_pos, const SDL_FPoint& target_pos, BlockType type);

    [[nodiscard]] bool IsAlive() const { return is_alive_; }
    [[nodiscard]] bool IsAttacking() const { return is_attacking_; }
    [[nodiscard]] State GetState() const { return current_state_; }
    [[nodiscard]] BlockType GetBlockType() const { return block_type_; }
    [[nodiscard]] int16_t GetBlockCount() const { return block_count_; }

    void SetAttacking(bool attacking) { is_attacking_ = attacking; }
    void SetBlockCount(int16_t count) { block_count_ = count; }

private:
    struct TextureRects 
    {
        SDL_FRect head{};     // 총알 머리 부분
        SDL_FRect tail{};     // 총알 꼬리 부분
        std::array<SDL_FRect, 2> shock{};  // 충격파 효과
    } rects_;


    // Resources
    std::shared_ptr<ImageTexture> texture_;

    // Movement
    SDL_FPoint start_pos_{};
    SDL_FPoint target_pos_{};
    SDL_FPoint velocity_{};
    SDL_FPoint direction_{};
    SDL_FPoint previous_pos_{};
    SDL_FPoint shock_pos_{};
    SDL_FPoint rotation_center_{};

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