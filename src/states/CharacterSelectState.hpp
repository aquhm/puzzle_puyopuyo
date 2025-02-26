#pragma once

#include "BaseState.hpp"
#include "../network/PacketProcessor.hpp"
#include "../network/packets/GamePackets.hpp"

#include <memory>
#include <array>
#include <string>
#include <span>


class ImageTexture;
class Button;
struct SDL_Point;

struct CharacterImageInfo 
{
    uint8_t characterIndex{ 0 };
    int x{ 0 }, y{ 0 };
    std::shared_ptr<ImageTexture> smallPortrait;
    std::shared_ptr<ImageTexture> largePortrait;

    CharacterImageInfo() = default;
};

class CharacterSelectState final : public BaseState 
{
public:
    // 상수 정의
    static constexpr int CHAR_SELECT_BG_POS_X = 92;
    static constexpr int CHAR_SELECT_BG_POS_Y = 65;
    static constexpr int CHAR_SELECT_BG_SPACING = 3;
    static constexpr int GAME_CHARACTER_COUNT = 22;

    CharacterSelectState();
    ~CharacterSelectState() override = default;

    CharacterSelectState(const CharacterSelectState&) = delete;
    CharacterSelectState& operator=(const CharacterSelectState&) = delete;
    CharacterSelectState(CharacterSelectState&&) = delete;
    CharacterSelectState& operator=(CharacterSelectState&&) = delete;

   bool Init() override;
   void Enter() override;
   void Leave() override;
   void Update(float deltaTime) override;
   void Render() override;
   void Release() override;
   void HandleEvent(const SDL_Event& event) override;
   void HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) override;
   [[nodiscard]] std::string_view GetStateName() const override { return "CharSelect"; }

    // 게임 상태 제어
    bool RequireGameStart();
    void SetEnemySelectPos(uint8_t x, uint8_t y);
    void SetEnemyDecide(uint8_t x, uint8_t y);

private:

    [[nodiscard]] bool CanMoveDown();
    [[nodiscard]] bool CanMoveLeft();
    [[nodiscard]] bool CanMoveRight();

    void HandleKeyInput(SDL_Keycode key);
    void RenderCharacterPreviews();
    void RenderCharacterGrid();
    void RenderSelectionUI();
    void SelectRandomCharacter();
    void UpdateStartButton();    

    bool LoadBackgrounds();
    bool LoadCharacterResources();
    bool LoadUIElements();

    //void UpdateSelection(const SDL_Event& event);
    void HandleCharacterSelection();

    void HandleChangeCharSelect(uint8_t connectionId, const ChangeCharSelectPacket* packet);
    void HandleDecideCharSelect(uint8_t connectionId, const DecideCharacterPacket* packet);
    void HandleStartGame(uint8_t connectionId, const PacketBase* packet);

    void InitializePacketHandlers();

private:
    // 리소스 관리
    std::array<std::shared_ptr<ImageTexture>, 2> backgrounds_;
    std::shared_ptr<ImageTexture> select_background_;
    std::shared_ptr<ImageTexture> selection_box_;
    std::shared_ptr<ImageTexture> player_cursor_;
    std::shared_ptr<ImageTexture> enemy_cursor_;
    std::shared_ptr<ImageTexture> decide_marker_;
    std::shared_ptr<Button> start_button_;

    // 캐릭터 정보 관리
    std::array<std::unique_ptr<CharacterImageInfo>, 4 * 7> character_info_;


    SDL_Point current_pos_;
    SDL_Point enemy_pos_;
    bool is_selected_{ false };
    bool is_enemy_selected_{ false };

    PacketProcessor packet_processor_{};
};