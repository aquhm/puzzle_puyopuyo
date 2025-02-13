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

    // 복사/이동 생성자 명시적 삭제 
    LoginState(const LoginState&) = delete;
    LoginState& operator=(const LoginState&) = delete;
    LoginState(LoginState&&) = delete;
    LoginState& operator=(LoginState&&) = delete;

    // BaseState 인터페이스 구현
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
            int posX{ 0 };  // 화면 중앙 기준으로 계산
            int posY{ 200 };
        } textBox;

        struct {
            int width{ DEFAULT_BUTTON_WIDTH };
            int height{ DEFAULT_BUTTON_HEIGHT };
            int loginPosY{ 230 };
            int createServerPosY{ 300 };
        } buttons;
    };

    // UI 구성요소들
    std::array<std::unique_ptr<Texture>, BACKGROUND_COUNT> backgrounds;
    std::unique_ptr<Button> loginButton;
    std::unique_ptr<Button> createServerButton;
    std::unique_ptr<TextBox> ipAddressInput;
    UIConfig uiConfig;

    // 내부 메서드들
    bool initializeBackgrounds();
    bool initializeUI();
    bool loadResources();

    // 이벤트 핸들러
    void handleKeyboardEvent(const SDL_Event& event);
    void handleNetworkEvent(const SDL_SysWMmsg* msg);

    // 네트워크 관련
    bool requestConnection();
    bool requestGameServerCreation();

    // 유틸리티
    void centerUIElements();
    void updateUIPositions();
};