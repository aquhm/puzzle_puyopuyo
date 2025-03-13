#pragma once
/*
 *
 * 설명:	방 상태
 *
 */

#include "BaseState.hpp"
#include "../network/PacketProcessor.hpp"
#include "../network/packets/GamePackets.hpp"
#include "../ui/EditBox.hpp"
#include "../ui/Label.hpp"

#include <array>
#include <memory>
#include <string_view>


class ImageTexture;
class Button;
class PacketProcessor;

class RoomState final : public BaseState
{
public:
    RoomState();
    ~RoomState() override = default;

    RoomState(const RoomState&) = delete;
    RoomState& operator=(const RoomState&) = delete;
    RoomState(RoomState&&) = delete;
    RoomState& operator=(RoomState&&) = delete;

    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Release()override;
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) override;

    [[nodiscard]] std::string_view GetStateName() const override
    {
        return "Room";
    }

    [[nodiscard]] EditBox* GetChatBox() const { return ui_elements_.chat_box.get(); }

private:

    bool LoadBackgrounds();
    bool InitializeUI();

    void HandleMouseEvent(const SDL_Event& event);
    void HandleKeyboardEvent(const SDL_Event& event);

    // 게임 로직
    bool StartGame();
    bool ExitGame();
    bool SendChatMessage();

    // 네트워크 메시지 처리
    void HandleChatMessage(uint8_t connectionId, const ChatMessagePacket* packet);
    void HandlePlayerJoined(uint8_t connectionId, const AddPlayerPacket* packet);
    void HandlePlayerLeft(uint8_t connectionId, const RemovePlayerInRoomPacket* packet);
    void HandleGameStart(uint8_t connectionId, const PacketBase* packet);

    void RenderBackground() const;
    void RenderUI() const;
    void UpdateBackgroundAnimation(float deltaTime);
    void InitializePacketHandlers();

private:
    static constexpr size_t BACKGROUND_COUNT = 10;
    static constexpr float BACKGROUND_SCROLL_SPEED = 50.0f;

    struct BackgroundAnimation 
    {
        uint8_t render_index{ 0 };
        float scroll_offset{ 0.0f };

        void Reset() {
            render_index = 0;
            scroll_offset = 0.0f;
        }
    } background_animation_;

    struct UIElements 
    {
        std::unique_ptr<Label> chat_label;
        std::unique_ptr<EditBox> chat_box;
        std::unique_ptr<Button> start_button;
        std::unique_ptr<Button> exit_button;
    } ui_elements_;

    std::array<std::shared_ptr<ImageTexture>, BACKGROUND_COUNT> backgrounds_;

    PacketProcessor packet_processor_{};

};