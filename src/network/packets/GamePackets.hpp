#pragma once
#include "PacketBase.hpp"
#include <array>
#include <cstdint>
#include <algorithm>
#include <string_view>

#pragma pack(push, 1)

// 채팅 관련
struct ChatMessagePacket : PacketBase
{
    uint8_t player_id{};
    std::array<char, 151> message{};

    ChatMessagePacket()
    {
        type = static_cast<uint16_t>(PacketType::ChatMessage);
        size = sizeof(ChatMessagePacket);
    }

    void SetMessage(std::string_view msg)
    {
        auto length = std::min<size_t>(msg.length(), message.size() - 1);
        std::copy_n(msg.data(), length, message.data());
        message[length] = '\0';
    }
};

// 캐릭터 선택 관련
struct ChangeCharSelectPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t x_pos{};
    uint8_t y_pos{};

    ChangeCharSelectPacket()
    {
        type = static_cast<uint16_t>(PacketType::ChangeCharSelect);
        size = sizeof(ChangeCharSelectPacket);
    }
};

struct DecideCharacterPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t x_pos{};
    uint8_t y_pos{};

    DecideCharacterPacket()
    {
        type = static_cast<uint16_t>(PacketType::DecideCharSelect);
        size = sizeof(DecideCharacterPacket);
    }
};

// 게임 초기화/블록 관련
struct GameInitPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t map_id{};
    uint16_t character_id{};
    std::array<uint8_t, 2> block1{};
    std::array<uint8_t, 2> block2{};

    GameInitPacket()
    {
        type = static_cast<uint16_t>(PacketType::InitializeGame);
        size = sizeof(GameInitPacket);
    }
};

struct MoveBlockPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t move_type{};
    float position{};

    MoveBlockPacket()
    {
        type = static_cast<uint16_t>(PacketType::UpdateBlockMove);
        size = sizeof(MoveBlockPacket);
    }
};

struct RotateBlockPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t rotate_type{};
    bool is_horizontal_moving{};

    RotateBlockPacket()
    {
        type = static_cast<uint16_t>(PacketType::UpdateBlockRotate);
        size = sizeof(RotateBlockPacket);
    }
};

// 공격/방어 관련
struct AttackInterruptPacket : PacketBase
{
    uint8_t player_id{};
    int16_t count{};
    float position_x{};
    float position_y{};
    uint8_t block_type{};

    AttackInterruptPacket()
    {
        type = static_cast<uint16_t>(PacketType::AttackInterruptBlock);
        size = sizeof(AttackInterruptPacket);
    }
};


struct DefenseInterruptPacket : PacketBase
{
    uint8_t player_id{};
    int16_t count{};
    float position_x{};
    float position_y{};
    uint8_t block_type{};

    DefenseInterruptPacket()
    {
        type = static_cast<uint16_t>(PacketType::DefenseInterruptBlock);
        size = sizeof(DefenseInterruptPacket);
    }
};

struct AddInterruptBlockPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t y_row_count{};
    uint8_t x_count{};
    std::array<uint8_t, 5> x_indices{};

    AddInterruptBlockPacket()
    {
        type = static_cast<uint16_t>(PacketType::AddInterruptBlock);
        size = sizeof(AddInterruptBlockPacket);
    }
};

// 게임 상태 관련
struct CheckBlockStatePacket : PacketBase
{
    uint8_t player_id{};

    CheckBlockStatePacket()
    {
        type = static_cast<uint16_t>(PacketType::CheckBlockState);
        size = sizeof(CheckBlockStatePacket);
    }
};

struct UpdateBlockPosPacket : PacketBase
{
    uint8_t player_id{};
    float position1{};
    float position2{};

    UpdateBlockPosPacket()
    {
        type = static_cast<uint16_t>(PacketType::UpdateBlockPos);
        size = sizeof(UpdateBlockPosPacket);
    }
};

struct FallingBlockPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t falling_index{};
    bool is_falling{};

    FallingBlockPacket()
    {
        type = static_cast<uint16_t>(PacketType::UpdateBlockFalling);
        size = sizeof(FallingBlockPacket);
    }
};

struct ChangeBlockStatePacket : PacketBase
{
    uint8_t player_id{};
    uint8_t state{};

    ChangeBlockStatePacket()
    {
        type = static_cast<uint16_t>(PacketType::ChangeBlockState);
        size = sizeof(ChangeBlockStatePacket);
    }
};

struct PushBlockPacket : PacketBase
{
    uint8_t player_id{};
    std::array<float, 2> position1{};
    std::array<float, 2> position2{};

    PushBlockPacket()
    {
        type = static_cast<uint16_t>(PacketType::PushBlockInGame);
        size = sizeof(PushBlockPacket);
    }
};

// 게임 진행/종료 관련
struct StopComboPacket : PacketBase
{
    uint8_t player_id{};

    StopComboPacket()
    {
        type = static_cast<uint16_t>(PacketType::StopComboAttack);
        size = sizeof(StopComboPacket);
    }
};

struct LoseGamePacket : PacketBase
{
    uint8_t player_id{};

    LoseGamePacket()
    {
        type = static_cast<uint16_t>(PacketType::LoseGame);
        size = sizeof(LoseGamePacket);
    }
};

struct StartGamePacket : PacketBase
{
    StartGamePacket()
    {
        type = static_cast<uint16_t>(PacketType::StartGame);
        size = sizeof(StartGamePacket);
    }
};

struct GameOverPacket : PacketBase
{
    GameOverPacket()
    {
        type = static_cast<uint16_t>(PacketType::GameOver);
        size = sizeof(GameOverPacket);
    }
};

struct RestartGamePacket : PacketBase
{
    uint8_t player_id{};
    uint8_t map_id{};
    std::array<uint8_t, 2> block1{};
    std::array<uint8_t, 2> block2{};

    RestartGamePacket()
    {
        type = static_cast<uint16_t>(PacketType::RestartGame);
        size = sizeof(RestartGamePacket);
    }
};

struct InitializePlayerPacket : PacketBase
{
    uint8_t player_id{};
    uint16_t character_idx{};
    std::array<uint8_t, 2> block_type1{};
    std::array<uint8_t, 2> block_type2{};

    InitializePlayerPacket()
    {
        type = static_cast<uint16_t>(PacketType::InitializePlayer);
        size = sizeof(InitializePlayerPacket);
    }
};

struct AddNewBlockPacket : PacketBase
{
    uint8_t player_id{};
    std::array<uint8_t, 2> block_type{};  // 블록 타입 2개

    AddNewBlockPacket()
    {
        type = static_cast<uint16_t>(PacketType::AddNewBlock);
        size = sizeof(AddNewBlockPacket);
    }
};

struct ComboPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t combo_count{};        // 현재 콤보 카운트
    float combo_position_x{};     // 콤보 발생 위치 x
    float combo_position_y{};     // 콤보 발생 위치 y
    bool is_continue{};           // 콤보가 계속되는지 여부

    ComboPacket()
    {
        type = static_cast<uint16_t>(PacketType::ComboUpdate);
        size = sizeof(ComboPacket);
    }
};

struct GiveIdPacket : PacketBase
{
    uint8_t player_id{};

    GiveIdPacket()
    {
        type = static_cast<uint16_t>(PacketType::GiveId);
        size = sizeof(GiveIdPacket);
    }
};

struct StartCharSelectPacket : PacketBase
{
    StartCharSelectPacket()
    {
        type = static_cast<uint16_t>(PacketType::StartCharSelect);
        size = sizeof(StartCharSelectPacket);
    }
};

struct RemovePlayerPacket : PacketBase
{
    uint8_t player_id{};

    RemovePlayerPacket()
    {
        type = static_cast<uint16_t>(PacketType::RemovePlayer);
        size = sizeof(RemovePlayerPacket);
    }
};

struct PlayerInfoPacket : PacketBase
{
    uint8_t player_id{};
    uint8_t character_id{};

    PlayerInfoPacket()
    {
        type = static_cast<uint16_t>(PacketType::PlayerInfo);
        size = sizeof(PlayerInfoPacket);
    }
};

struct AddPlayerPacket : PacketBase
{
    uint8_t player_id{};
    uint16_t character_id{};

    AddPlayerPacket()
    {
        type = static_cast<uint16_t>(PacketType::AddPlayer);
        size = sizeof(AddPlayerPacket);
    }
};

struct RemovePlayerInRoomPacket : PacketBase
{
    uint8_t id{};

    RemovePlayerInRoomPacket()
    {
        type = static_cast<uint16_t>(PacketType::RemovePlayerInRoom);
        size = sizeof(RemovePlayerInRoomPacket);
    }
};

struct ConnectLobbyPacket : PacketBase
{
    uint8_t id{};

    ConnectLobbyPacket()
    {
        type = static_cast<uint16_t>(PacketType::ConnectLobby);
        size = sizeof(ConnectLobbyPacket);
    }
};

struct DefenseResultInterruptBlockCountPacket : PacketBase
{
    uint8_t player_id{};
    uint16_t count{};

    DefenseResultInterruptBlockCountPacket()
    {
        type = static_cast<uint16_t>(PacketType::DefenseResultInterruptBlockCount);
        size = sizeof(DefenseResultInterruptBlockCountPacket);
    }
};

struct AttackResultPlayerInterruptBlocCountPacket : PacketBase
{
    uint8_t player_id{};
    uint16_t count{};
    uint16_t attackerCount{};

    AttackResultPlayerInterruptBlocCountPacket()
    {
        type = static_cast<uint16_t>(PacketType::AttackResultPlayerInterruptBlocCount);
        size = sizeof(AttackResultPlayerInterruptBlocCountPacket);
    }
};

struct SyncBlockPositionYPacket : public PacketBase
{
    uint8_t player_id{};
    float position_y{};
    float velocity{};

    SyncBlockPositionYPacket()
    {
        type = static_cast<uint16_t>(PacketType::SyncBlockPositionY);
        size = sizeof(SyncBlockPositionYPacket);
    }
};

#pragma pack(pop)