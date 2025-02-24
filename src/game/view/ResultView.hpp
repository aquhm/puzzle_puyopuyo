#pragma once
/*
 *
 * 설명: 게임 승패 표시
 *
 */

#include "../RenderableObject.hpp"
#include <memory>

class ImageTexture;

class ResultView final : public RenderableObject 
{

private:
    static constexpr float ROTATION_SPEED = 100.0f;
    static constexpr float OSCILLATION_AMPLITUDE = 30.0f;
    static constexpr float FULL_ROTATION = 360.0f;

    static constexpr int TEXTURE_WIDTH = 158;
    static constexpr int TEXTURE_HEIGHT = 98;
    static constexpr int WIN_TEXTURE_Y = 0;
    static constexpr int LOSE_TEXTURE_Y = 96;

public:

    ResultView() = default;
    ~ResultView() override = default;
    
    ResultView(const ResultView&) = delete;
    ResultView& operator=(const ResultView&) = delete;
    ResultView(ResultView&&) noexcept = delete;
    ResultView& operator=(ResultView&&) noexcept = delete;

    bool Initialize();
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;
    void UpdateResult(float xPos, float yPos, bool isWin);

private:

    void UpdateVerticalPosition();

private:
    SDL_FRect resultRect_{};
    std::shared_ptr<ImageTexture> resultTexture_;

    float originalPosY_{ 0.0f };
    float rotationAngle_{ 0.0f };
};