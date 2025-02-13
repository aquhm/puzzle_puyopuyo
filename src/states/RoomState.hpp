// states/RoomState.hpp
#pragma once

#include "BaseState.hpp"
#include <array>
#include <memory>
#include <chrono>

class Texture;
class EditBox;
class Button;

class RoomState final : public BaseState {
public:
    RoomState() = default;
    ~RoomState() override = default;

    // 복사/이동 방지
    RoomState(const RoomState&) = delete;
    RoomState& operator=(const RoomState&) = delete;
    RoomState(RoomState&&) = delete;
    RoomState& operator=(RoomState&&) = delete;

    // BaseState 인터페이스 구현
    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    /*void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;*/

    [[nodiscard]] std::string_view getStateName() const override {
        return "Room";
    }

    // 채팅 관련
    [[nodiscard]] EditBox* getChatBox() const { return chatBox.get(); }
    bool sendChatMessage();

private:
    static constexpr size_t BACKGROUND_COUNT = 10;
    static constexpr float BACKGROUND_SCROLL_SPEED = 50.0f;

    struct BackgroundAnimation {
        uint8_t renderIndex{ 0 };
        float scrollOffset{ 0.0f };

        void reset() {
            renderIndex = 0;
            scrollOffset = 0.0f;
        }
    };

    struct UIElements {
        std::unique_ptr<EditBox> chatBox;
        std::unique_ptr<Button> startButton;
        std::unique_ptr<Button> exitButton;
    };

    // 멤버 변수들
    std::array<std::unique_ptr<Texture>, BACKGROUND_COUNT> backgrounds;
    UIElements ui;
    BackgroundAnimation bgAnimation;

    // 초기화 메서드들
    bool initializeBackgrounds();
    bool initializeUI();
    bool loadResources();

    // 이벤트 처리
    void handleKeyboardEvent(const SDL_Event& event);
    void handleNetworkEvent(const SDL_SysWMmsg* msg);
    void handleChatInput(const SDL_Event& event);

    // 게임 로직
    bool startGame();
    bool exitGame();
    void updateBackgroundAnimation(float deltaTime);
    void renderBackground() const;

    // 네트워크 메시지 처리
    void handleChatMessage(const std::string_view& message);
    void handlePlayerJoined(uint32_t playerId);
    void handlePlayerLeft(uint32_t playerId);
    void handleGameStart();
};