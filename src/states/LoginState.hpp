#pragma once
/*
 *
 * 설명: 로그인 상태
 *
 */
#include "BaseState.hpp"
#include "../ui/Button.hpp"
#include "../ui/Label.hpp"
#include "../ui/TextBox.hpp"
#include "../network/PacketProcessor.hpp"
#include "../network/packets/GamePackets.hpp"

#include <array>
#include <memory>

class ImageTexture;
class TextBox;
class Label;
class PacketProcessor;

class LoginState final : public BaseState
{
public:

    static constexpr size_t BACKGROUND_COUNT = 2;

    LoginState();
    ~LoginState() override = default;

    LoginState(const LoginState&) = delete;
    LoginState& operator=(const LoginState&) = delete;
    LoginState(LoginState&&) = delete;
    LoginState& operator=(LoginState&&) = delete;

    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Release()override;
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) override;

    [[nodiscard]] std::string_view GetStateName() const override { return "Login"; }

private:

    void RenderUI() const;
    void RenderBackground() const;

    void HandleMouseEvent(const SDL_Event& event);
    void HandleKeyboardEvent(const SDL_Event& event);

    bool LoadBackgrounds();
    bool InitializeUI();

    bool RequireConnect();      // 서버 접속 요청
    bool RequireInitGameSrv();  // 게임 서버 생성
    void HandleGiveId(uint8_t playerId);

    void InitializePacketHandlers();

private:    

    struct UIElements 
    {
        std::unique_ptr<Label> ip_label;
        std::unique_ptr<TextBox> ip_input;
        std::unique_ptr<Button> login_button;
        std::unique_ptr<Button> create_server_button;
    } ui_elements_;

    std::array<std::shared_ptr<ImageTexture>, BACKGROUND_COUNT> backgrounds_;

    PacketProcessor packet_processor_{};
};