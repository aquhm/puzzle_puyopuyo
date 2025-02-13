#include "BulletEffect.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../block/Block.hpp"
#include <cmath>
#include <stdexcept>

namespace effects {

    BulletEffect::BulletEffect() {
        // Initialize texture rects
        rects_.head.w = 191;
        rects_.head.h = 191;

        rects_.tail.w = 65;
        rects_.tail.h = 191;

        rects_.shock[0] = { 0, 382, 256, 256 };
        rects_.shock[1] = { 256, 382, 256, 256 };

        // Initialize rotation center
        rotation_center_ = { rects_.tail.w / 2, 0 };
    }

    bool BulletEffect::Initialize(const SDL_FPoint& start_pos,
        const SDL_FPoint& target_pos,
        BlockType type) {
        texture_ = RESOURCE_MANAGER.GetTexture("./Image/PUYO/Effect/effect.png");
        if (!texture_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load bullet effect texture");
            return false;
        }

        // Set blending mode
        texture_->SetBlendMode(SDL_BLENDMODE_ADD);
        texture_->SetAlpha(255);

        shock_pos_ = target_pos;

        // 발사체의 중심점으로 설정
        SDL_FPoint adjusted_start = start_pos;
        SDL_FPoint adjusted_target = target_pos;
        adjusted_start.x -= rects_.head.w / 2.0f;
        adjusted_start.y -= rects_.head.h / 2.0f;
        adjusted_target.x -= rects_.head.w / 2.0f;
        adjusted_target.y -= rects_.head.h / 2.0f;

        // 각도 및 방향 벡터 계산
        float angle = std::atan2(adjusted_start.y - adjusted_target.y,
            adjusted_target.x - adjusted_start.x);
        angle = ToDegrees(angle);

        direction_.x = BULLET_FORCE * std::cos(ToRadians(angle));
        direction_.y = BULLET_FORCE * -std::sin(ToRadians(angle));

        start_pos_ = adjusted_start;
        target_pos_ = adjusted_target;
        previous_pos_ = start_pos_;

        block_type_ = type;
        SetupTextureRects();
        SetPosition(start_pos_.x, start_pos_.y);

        accumulated_time_ = 0.0f;
        is_alive_ = true;
        current_state_ = State::Moving;

        return true;
    }

    void BulletEffect::SetupTextureRects() {
        switch (block_type_) {
        case BlockType::Red:
            rects_.head.x = 256;
            rects_.head.y = 0;
            break;
        case BlockType::Green:
            rects_.head.x = 256 * 2;
            rects_.head.y = 0;
            break;
        case BlockType::Blue:
            rects_.head.x = 256 * 3;
            rects_.head.y = 0;
            break;
        case BlockType::Yellow:
            rects_.head.x = 0;
            rects_.head.y = 191;
            break;
        case BlockType::Purple:
            rects_.head.x = 256;
            rects_.head.y = 191;
            break;
        default:
            throw std::runtime_error("Invalid block type for bullet effect");
        }
        rects_.tail.x = rects_.head.x + 191;
        rects_.tail.y = rects_.head.y;
    }

    void BulletEffect::Update(float delta_time) {
        if (!is_alive_) return;

        accumulated_time_ += delta_time;

        switch (current_state_) {
        case State::Moving:
            UpdateMovement(delta_time);
            break;
        case State::Shocking:
            UpdateShockwave(delta_time);
            break;
        case State::Out:
            is_alive_ = false;
            break;
        }
    }

    void BulletEffect::UpdateMovement(float delta_time) {
        if (accumulated_angle_ >= 180.0f) {
            current_state_ = State::Shocking;
            SetSize(scale_, scale_);
            SetPosition(shock_pos_.x - (scale_ / 2), shock_pos_.y - (scale_ / 2));
            accumulated_time_ = 0.0f;
            return;
        }

        velocity_.x = direction_.x * accumulated_time_;
        velocity_.y = direction_.y * accumulated_time_;

        accumulated_angle_ = (180.0f * velocity_.x) / (target_pos_.x - start_pos_.x);
        float y = BULLET_AMPLITUDE * -std::sin(ToRadians(accumulated_angle_));
        velocity_.y += y;

        float new_x = start_pos_.x + velocity_.x;
        float new_y = start_pos_.y + velocity_.y;

        SetPosition(new_x, new_y);

        // 꼬리 회전 각도 계산
        float dx = new_x - previous_pos_.x;
        float dy = new_y - previous_pos_.y;
        tail_angle_ = std::atan2(dy, dx) * 180.0f / PI + 90.0f;

        previous_pos_ = { new_x, new_y };
    }

    void BulletEffect::UpdateShockwave(float delta_time) {
        if (accumulated_time_ <= SHOCK_EXPAND_TIME) {
            float scale_velocity = SHOCK_EXPAND_DELTA_SIZE / SHOCK_EXPAND_TIME * delta_time;
            float pos_velocity = SHOCK_EXPAND_POS_VELOCITY * delta_time;

            SDL_FRect current_rect = GetRect();
            current_rect.w += scale_velocity;
            current_rect.h += scale_velocity;
            current_rect.x -= pos_velocity;
            current_rect.y -= pos_velocity;

            SetPosition(current_rect.x, current_rect.y);
            SetSize(current_rect.w, current_rect.h);

            alpha_ = static_cast<uint8_t>(std::max(0.0f,
                static_cast<float>(alpha_) - (pos_velocity * 10)));
            if (texture_) {
                texture_->SetAlpha(alpha_);
            }
        }
        else {
            is_alive_ = false;
        }
    }

    void BulletEffect::Render() {
        if (!is_visible_ || !is_alive_ || !texture_) return;

        switch (current_state_) {
        case State::Moving:
            RenderMoving();
            break;
        case State::Shocking:
            RenderShockwave();
            break;
        default:
            break;
        }
    }

    void BulletEffect::RenderMoving() const {
        // 꼬리부터 그려서 머리가 위에 오도록 함
        texture_->Render(
            static_cast<int>(GetX() + 63),
            static_cast<int>(GetY() + 95.5),
            &rects_.tail,
            tail_angle_,
            &rotation_center_
        );
        texture_->Render(
            static_cast<int>(GetX()),
            static_cast<int>(GetY()),
            &rects_.head
        );
    }

    void BulletEffect::RenderShockwave() const {
        const SDL_FRect& current_rect = GetRect();
        SDL_Rect dest_rect = {
            static_cast<int>(current_rect.x),
            static_cast<int>(current_rect.y),
            static_cast<int>(current_rect.w),
            static_cast<int>(current_rect.h)
        };

        texture_->ScaledRender(&rects_.shock[0], &dest_rect);
        texture_->ScaledRender(&rects_.shock[1], &dest_rect);
    }

    void BulletEffect::Release() {
        if (texture_) {
            texture_->SetAlpha(255);
        }
        texture_.reset();
    }

} // namespace effects