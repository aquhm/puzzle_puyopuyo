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

    // ����/�̵� ����
    RoomState(const RoomState&) = delete;
    RoomState& operator=(const RoomState&) = delete;
    RoomState(RoomState&&) = delete;
    RoomState& operator=(RoomState&&) = delete;

    // BaseState �������̽� ����
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

    // ä�� ����
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

    // ��� ������
    std::array<std::unique_ptr<Texture>, BACKGROUND_COUNT> backgrounds;
    UIElements ui;
    BackgroundAnimation bgAnimation;

    // �ʱ�ȭ �޼����
    bool initializeBackgrounds();
    bool initializeUI();
    bool loadResources();

    // �̺�Ʈ ó��
    void handleKeyboardEvent(const SDL_Event& event);
    void handleNetworkEvent(const SDL_SysWMmsg* msg);
    void handleChatInput(const SDL_Event& event);

    // ���� ����
    bool startGame();
    bool exitGame();
    void updateBackgroundAnimation(float deltaTime);
    void renderBackground() const;

    // ��Ʈ��ũ �޽��� ó��
    void handleChatMessage(const std::string_view& message);
    void handlePlayerJoined(uint32_t playerId);
    void handlePlayerLeft(uint32_t playerId);
    void handleGameStart();
};