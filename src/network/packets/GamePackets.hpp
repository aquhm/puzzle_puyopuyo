#pragma once
#include "PacketBase.hpp"
#include "PacketType.hpp"
#include <array>
#include <cstdint>
#include <algorithm>
#include <string_view>


#pragma pack(push, 1)

// 채팅 관련
struct ChatMessagePacket : PacketBase
{
    uint8_t player_id;
    std::array<char, 151> message;

    PacketType GetType() const override { return PacketType::ChatMessage; }
    uint32_t GetBodySize() const override { return sizeof(ChatMessagePacket); }

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
    uint8_t player_id;
    uint8_t x_pos;
    uint8_t y_pos;

    PacketType GetType() const override { return PacketType::ChangeCharSelect; }
    uint32_t GetBodySize() const override { return sizeof(ChangeCharSelectPacket); }
};

// 캐릭터 선택 관련
struct DecideCharacterPacket : PacketBase
{
    uint8_t player_id;
    uint8_t x_pos;
    uint8_t y_pos;

    PacketType GetType() const override { return PacketType::DecideCharSelect; }
    uint32_t GetBodySize() const override { return sizeof(DecideCharacterPacket); }
};

// 게임 초기화/블록 관련
struct GameInitPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t	map_id;
    uint8_t character_id;
    std::array<uint8_t, 2> block1;
    std::array<uint8_t, 2> block2;

    PacketType GetType() const override { return PacketType::InitializeGame; }
    uint32_t GetBodySize() const override { return sizeof(GameInitPacket); }
};

struct MoveBlockPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t move_type;
    float position;

    PacketType GetType() const override { return PacketType::UpdateBlockMove; }
    uint32_t GetBodySize() const override { return sizeof(MoveBlockPacket); }
};

struct RotateBlockPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t rotate_type;
    bool is_horizontal_moving;

    PacketType GetType() const override { return PacketType::UpdateBlockRotate; }
    uint32_t GetBodySize() const override { return sizeof(RotateBlockPacket); }
};

// 공격/방어 관련
struct AttackInterruptPacket : PacketBase 
{
    uint8_t player_id;
    int16_t count;
    float position_x;
    float position_y;
    uint8_t block_type;

    PacketType GetType() const override { return PacketType::AttackInterruptBlock; }
    uint32_t GetBodySize() const override { return sizeof(AttackInterruptPacket); }
};

struct DefenseInterruptPacket : PacketBase 
{
    uint8_t player_id;
    int16_t count;
    float position_x;
    float position_y;
    uint8_t block_type;

    PacketType GetType() const override { return PacketType::DefenseInterruptBlock; }
    uint32_t GetBodySize() const override { return sizeof(DefenseInterruptPacket); }
};

struct AddInterruptBlockPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t y_row_count;
    uint8_t x_count;
    std::array<uint8_t, 5> x_indices;

    PacketType GetType() const override { return PacketType::AddInterruptBlock; }
    uint32_t GetBodySize() const override { return sizeof(AddInterruptBlockPacket); }
};


// 게임 상태 관련
struct CheckBlockStatePacket : PacketBase 
{
    uint8_t player_id;

    PacketType GetType() const override { return PacketType::CheckBlockState; }
    uint32_t GetBodySize() const override { return sizeof(CheckBlockStatePacket); }
};

struct UpdateBlockPosPacket : PacketBase 
{
    uint8_t player_id;
    float position1;
    float position2;

    PacketType GetType() const override { return PacketType::UpdateBlockPos; }
    uint32_t GetBodySize() const override { return sizeof(UpdateBlockPosPacket); }
};

struct FallingBlockPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t falling_index;
    bool is_falling;

    PacketType GetType() const override { return PacketType::UpdateBlockFalling; }
    uint32_t GetBodySize() const override { return sizeof(FallingBlockPacket); }
};

struct ChangeBlockStatePacket : PacketBase 
{
    uint8_t player_id;
    uint8_t state;

    PacketType GetType() const override { return PacketType::ChangeBlockState; }
    uint32_t GetBodySize() const override { return sizeof(ChangeBlockStatePacket); }
};

struct PushBlockPacket : PacketBase 
{
    uint8_t player_id;
    std::array<float, 2> position1;
    std::array<float, 2> position2;

    PacketType GetType() const override { return PacketType::PushBlockInGame; }
    uint32_t GetBodySize() const override { return sizeof(PushBlockPacket); }
};

// 게임 진행/종료 관련
struct StopComboPacket : PacketBase 
{
    uint8_t player_id;

    PacketType GetType() const override { return PacketType::StopComboAttack; }
    uint32_t GetBodySize() const override { return sizeof(StopComboPacket); }
};

struct LoseGamePacket : PacketBase 
{
    uint8_t player_id;

    PacketType GetType() const override { return PacketType::LoseGame; }
    uint32_t GetBodySize() const override { return sizeof(LoseGamePacket); }
};

struct StartGamePacket : PacketBase
{      
    PacketType GetType() const override { return PacketType::StartGame; }
    uint32_t GetBodySize() const override { return sizeof(StartGamePacket); }
};


struct RestartGamePacket : PacketBase 
{
    uint8_t player_id;
    uint8_t	map_id;
    std::array<uint8_t, 2> block1;
    std::array<uint8_t, 2> block2;

    PacketType GetType() const override { return PacketType::RestartGame; }
    uint32_t GetBodySize() const override { return sizeof(RestartGamePacket); }
};

struct InitializePlayerPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t character_idx;
    std::array<uint8_t, 2> block_type1;
    std::array<uint8_t, 2> block_type2;

    PacketType GetType() const override { return PacketType::InitializePlayer; }
    uint32_t GetBodySize() const override { return sizeof(InitializePlayerPacket); }
};

struct AddNewBlockPacket : PacketBase 
{
    uint8_t player_id;
    std::array<uint8_t, 2> block_type;  // 블록 타입 2개

    PacketType GetType() const override { return PacketType::AddNewBlock; }
    uint32_t GetBodySize() const override { return sizeof(AddNewBlockPacket); }
};

struct ComboPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t combo_count;        // 현재 콤보 카운트
    float combo_position_x;     // 콤보 발생 위치 x
    float combo_position_y;     // 콤보 발생 위치 y
    bool is_continue;           // 콤보가 계속되는지 여부

    PacketType GetType() const override { return PacketType::ComboUpdate; }
    uint32_t GetBodySize() const override { return sizeof(ComboPacket); }
};


struct GiveIdPacket : PacketBase
{
    uint8_t player_id;      

    PacketType GetType() const override { return PacketType::GiveId; }
    uint32_t GetBodySize() const override { return sizeof(GiveIdPacket); }
};

struct StartCharSelectPacket : PacketBase
{
    PacketType GetType() const override { return PacketType::StartCharSelect; }
    uint32_t GetBodySize() const override { return sizeof(StartCharSelectPacket); }
};

struct RemovePlayerPacket : PacketBase 
{
    uint8_t player_id;

    PacketType GetType() const override { return PacketType::RemovePlayer; }
    uint32_t GetBodySize() const override { return sizeof(RemovePlayerPacket); }
};

struct PlayerInfoPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t character_id;

    PacketType GetType() const override { return PacketType::PlayerInfo; }
    uint32_t GetBodySize() const override { return sizeof(PlayerInfoPacket); }
};

struct AddPlayerPacket : PacketBase 
{
    uint8_t player_id;
    uint8_t character_id;

    PacketType GetType() const override { return PacketType::AddPlayer; }
    uint32_t GetBodySize() const override { return sizeof(AddPlayerPacket); }
};


struct RemovePlayerInRoomPacket : PacketBase 
{
    uint8_t id;

    PacketType GetType() const override { return PacketType::RemovePlayerInRoom; }
    uint32_t GetBodySize() const override { return sizeof(RemovePlayerInRoomPacket); }
};

struct ConnectLobbyPacket : PacketBase
{
    uint8_t id;

    PacketType GetType() const override { return PacketType::ConnectLobby; }
    uint32_t GetBodySize() const override { return sizeof(ConnectLobbyPacket); }
};

struct DefenseResultInterruptBlockCountPacket : PacketBase
{
    uint8_t id;
    uint16_t count;

    PacketType GetType() const override { return PacketType::DefenseResultInterruptBlockCount; }
    uint32_t GetBodySize() const override { return sizeof(DefenseResultInterruptBlockCountPacket); }
};

struct AttackResultPlayerInterruptBlocCountPacket : PacketBase
{
    uint8_t id;
    uint16_t count;
    uint16_t attackerCount;

    PacketType GetType() const override { return PacketType::AttackResultPlayerInterruptBlocCount; }
    uint32_t GetBodySize() const override { return sizeof(AttackResultPlayerInterruptBlocCountPacket); }
};

#pragma pack(pop)