#pragma once
#include <cstdint>


enum class PacketType : uint16_t 
{
    None = 0,

    // 연결/초기화 관련 (1-99)
    GiveId = 1,
    ConnectLobby = 2,

    //플레이어 관련
    RemovePlayer = 50,
    PlayerInfo = 51,
    AddPlayer = 52,
    RemovePlayerInRoom = 53,

    // 채팅 관련 (100-199)
    ChatMessage = 100,
    ChatMessageServer = 101,

    // 캐릭터 선택 관련 (200-299)
    StartCharSelect = 200,
    ChangeCharSelect = 201,
    DecideCharSelect = 202,

    // 게임 시작/초기화 관련 (300-399)
    StartGame = 300,
    InitializeGame = 301,
    InitializePlayer = 302,
    RestartGame = 303,
    GameOver = 304,

    // 블록 조작 관련 (400-499)
    AddNewBlock = 400,
    UpdateBlockState = 401,
    UpdateBlockMove = 402,
    UpdateBlockRotate = 403,
    CheckBlockState = 404,
    UpdateBlockPos = 405,
    UpdateBlockFalling = 406,
    ChangeBlockState = 407,
    PushBlockInGame = 408,
    SyncBlockPositionY = 409,

    // 공격/방어 관련 (500-599)
    AttackInterruptBlock = 500,
    DefenseInterruptBlock = 501,
    AddInterruptBlock = 502,
    StopComboAttack = 503,
    DefenseResultInterruptBlockCount = 504,
    AttackResultPlayerInterruptBlocCount = 505,

    // 500번대는 전투 관련이므로 거기에 배치
    ComboUpdate = 550,        

    // 게임 종료 관련 (600-699)
    LoseGame = 600,

    Max
};

// 패킷 타입 유효성 검사 헬퍼 함수
[[nodiscard]] constexpr bool IsValidPacketType(PacketType type) 
{
    return type > PacketType::None && type < PacketType::Max;
}

// 패킷 범위 확인 헬퍼 함수들
[[nodiscard]] constexpr bool IsConnectionPacket(PacketType type) 
{
    return type >= PacketType::GiveId && type < PacketType::ChatMessage;
}

[[nodiscard]] constexpr bool IsChatPacket(PacketType type) 
{
    return type >= PacketType::ChatMessage && type < PacketType::StartCharSelect;
}

[[nodiscard]] constexpr bool IsCharacterSelectPacket(PacketType type) 
{
    return type >= PacketType::StartCharSelect && type < PacketType::StartGame;
}

[[nodiscard]] constexpr bool IsGameInitPacket(PacketType type) 
{
    return type >= PacketType::StartGame && type < PacketType::AddNewBlock;
}

[[nodiscard]] constexpr bool IsBlockOperationPacket(PacketType type) 
{
    return type >= PacketType::AddNewBlock && type < PacketType::AttackInterruptBlock;
}

[[nodiscard]] constexpr bool IsCombatPacket(PacketType type) 
{
    return type >= PacketType::AttackInterruptBlock && type < PacketType::LoseGame;
}
