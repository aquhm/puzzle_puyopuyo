#pragma once

#include "../RenderableObject.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../particles/BgParticleSystem.hpp"
#include "../../core/common/constants/Constants.hpp"

#include <SDL3/SDL.h>
#include <array>
#include <deque>
#include <string>
#include <memory>
#include <map>

class ImgTexture;
class GroupBlock;

enum class BackgroundType : uint8_t
{
    GrassLand = 0,
    IceLand = 13
};

class GameBackground : public RenderableObject
{
public:

    static constexpr float MAX_DELTA_TIME = 0.05f; // 최대 50ms로 제한
    static constexpr size_t MAX_BLOCKS_IN_QUEUE = 3; // 큐에 허용되는 최대 블록 수

    GameBackground();
    ~GameBackground() override = default;

    GameBackground(const GameBackground&) = delete;
    GameBackground& operator=(const GameBackground&) = delete;
    GameBackground(GameBackground&&) = delete;
    GameBackground& operator=(GameBackground&&) = delete;

    [[nodiscard]] virtual bool Initialize();
    void Update(float deltaTime) override;
    [[nodiscard]] virtual void Render() override;
    void Release() override;

    void Reset();

    // 통합된 인터페이스
    void SetNextBlock(const std::shared_ptr<GroupBlock>& block, Constants::PlayerType playerType = Constants::PlayerType::Local);

    // 기존 호환성을 위한 메서드들
    void SetPlayerNextBlock(const std::shared_ptr<GroupBlock>& block) {
        SetNextBlock(block, Constants::PlayerType::Remote);
    }

    [[nodiscard]] bool IsChangingBlock(Constants::PlayerType playerType = Constants::PlayerType::Local) const;
    [[nodiscard]] bool IsChangingPlayerBlock() const {
        return IsChangingBlock(Constants::PlayerType::Remote);
    }

    [[nodiscard]] bool IsReadyGame() const;
    [[nodiscard]] uint8_t GetNewBlockCount(Constants::PlayerType playerType = Constants::PlayerType::Local) const;
    [[nodiscard]] uint8_t GetMapIndex() const { return map_index_; }

    // Setters
    void SetChangingBlock(bool state, Constants::PlayerType playerType = Constants::PlayerType::Local);
    void SetChangingPlayerBlock(bool state) {
        SetChangingBlock(state, Constants::PlayerType::Remote);
    }

protected:
    [[nodiscard]] virtual bool LoadBackgroundTextures();
    [[nodiscard]] bool CreateRenderTarget();
    void UpdateBlockAnimations(float deltaTime, Constants::PlayerType playerType);

    // 블록 애니메이션 상수 구조체
    struct BlockAnimationConfig 
    {
        SDL_FPoint direction_vector;
        float next_block_position_x;
        float next_block_position_y;
        int next_block_position_small_X;
        int next_block_position_small_y;
        float block_velocity;
        float scale_velocity;
    };

    // 플레이어 타입별 설정 가져오기
    BlockAnimationConfig GetAnimationConfig(Constants::PlayerType playerType) const;

protected:
    std::array<std::shared_ptr<ImageTexture>, 2> background_textures_;
    std::array<std::shared_ptr<ImageTexture>, 2> mask_textures_;
    std::array<SDL_FRect, 2> background_rects_{};

    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> render_target_;
    SDL_FRect render_target_rect_{};

    // 플레이어 타입별 블록 컨테이너 및 상태 관리
    struct PlayerData 
    {
        bool is_changing_block = false;
        std::deque<std::shared_ptr<GroupBlock>> group_blocks{};
        float accumulatedTime = 0.0f;
        SDL_FPoint direction_vector{};
    };

    std::map<Constants::PlayerType, PlayerData> player_data_;

    uint8_t map_index_{};
    bool is_initialized_{ false };
    float accumulated_time_{ 0.0f };
};