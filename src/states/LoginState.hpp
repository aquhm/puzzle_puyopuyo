#pragma once

#include "BaseState.hpp"
#include <array>
#include <string>
#include <memory>

class Texture;
class Button;
class TextBox;

class LoginState final : public BaseState {
public:
    LoginState() = default;
    ~LoginState() override = default;

    // ����/�̵� ������ ����� ���� 
    LoginState(const LoginState&) = delete;
    LoginState& operator=(const LoginState&) = delete;
    LoginState(LoginState&&) = delete;
    LoginState& operator=(LoginState&&) = delete;

    // BaseState �������̽� ����
    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    /*void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;*/

    [[nodiscard]] std::string_view getStateName() const override {
        return "Login";
    }

private:
    static constexpr size_t BACKGROUND_COUNT = 2;
    static constexpr int DEFAULT_BUTTON_WIDTH = 136;
    static constexpr int DEFAULT_BUTTON_HEIGHT = 49;

    struct UIConfig {
        struct {
            int width{ 150 };
            int height{ 23 };
            int posX{ 0 };  // ȭ�� �߾� �������� ���
            int posY{ 200 };
        } textBox;

        struct {
            int width{ DEFAULT_BUTTON_WIDTH };
            int height{ DEFAULT_BUTTON_HEIGHT };
            int loginPosY{ 230 };
            int createServerPosY{ 300 };
        } buttons;
    };

    // UI ������ҵ�
    std::array<std::unique_ptr<Texture>, BACKGROUND_COUNT> backgrounds;
    std::unique_ptr<Button> loginButton;
    std::unique_ptr<Button> createServerButton;
    std::unique_ptr<TextBox> ipAddressInput;
    UIConfig uiConfig;

    // ���� �޼����
    bool initializeBackgrounds();
    bool initializeUI();
    bool loadResources();

    // �̺�Ʈ �ڵ鷯
    void handleKeyboardEvent(const SDL_Event& event);
    void handleNetworkEvent(const SDL_SysWMmsg* msg);

    // ��Ʈ��ũ ����
    bool requestConnection();
    bool requestGameServerCreation();

    // ��ƿ��Ƽ
    void centerUIElements();
    void updateUIPositions();
};