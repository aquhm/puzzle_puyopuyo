#pragma once
/*
 *
 * 설명: 서버 네트워크 패킷 처리
 *
 */

#include "./NetServer.hpp"
#include "./CriticalSection.hpp"
#include "./packets/GamePackets.hpp"
#include "./packets/processors/IPacketProcessor.hpp"
#include "./packets/PacketType.hpp"
#include "../core/GameApp.hpp"
#include "../core/manager/PlayerManager.hpp"
#include "../network/player/Player.hpp"
#include "../utils/Logger.hpp"

#include <queue>
#include <memory>
#include <unordered_map>
#include <string_view>


struct ClientInfo;

struct ProcessEvent
{
    enum class Type 
    {
        Packet,
        Disconnect
    };

    Type event_type{ Type::Packet };
    std::span<const char> packet_data;
    ClientInfo* client_info{ nullptr };
    uint8_t player_id{ 0 };

    ProcessEvent() = default;

    // 패킷 이벤트 생성자
    ProcessEvent(std::span<const char> data, ClientInfo* client)
        : event_type(Type::Packet), packet_data(data), client_info(client) {
    }

    // 연결 해제 이벤트 생성자
    explicit ProcessEvent(uint8_t id)
        : event_type(Type::Disconnect), player_id(id) {
    }
};

class GameServer final : public NetServer 
{
public:
    GameServer();
    ~GameServer() override;

    using PacketProcessorPtr = std::unique_ptr<IPacketProcessor>;
    using PacketProcessorMap = std::unordered_map<PacketType, std::unique_ptr<IPacketProcessor>>;

    // 서버 제어
    bool StartServer();
    bool ExitServer();
    void Update();

    // 패킷 전송 헬퍼 함수
    template<typename PacketType> requires std::derived_from<PacketType, PacketBase>
    void BroadcastPacket(const PacketType& packet, uint8_t exclude_id = 0);
                

    // 게임 시작/초기화 관련
    void StartGame();
    void GameInitialize(std::span<const uint8_t> block1, std::span<const uint8_t> block2, uint8_t map_idx);
    void ReStartGame(std::span<const uint8_t> block1, std::span<const uint8_t> block2, uint8_t map_idx);
    void LoseGame();

    // 캐릭터 선택 관련
    void StartCharacterSelect();
    void ChangeCharSelect(uint8_t x, uint8_t y);
    void DecideCharacter(uint8_t x, uint8_t y);


    // 블록 조작 관련
    void AddNewBlock(std::span<const uint8_t> block);
    void MoveBlock(uint8_t moveType, float position);
    void RotateBlock(uint8_t rotateType, bool bHorizontalMoving);
    void CheckBlockState();
    void UpdateGameBlockPos(float fPos1, float fPos2);
    void RequireFallingBlock(uint8_t fallingIdx, bool falling);
    void ChangBlockState(uint8_t state);
    void PushBlockInGame(std::span<const float> pos1, std::span<const float> pos2);
    void SyncPositionY(float positionY, float velocity);

    // 전투/인터럽트 관련
    void AttackInterruptBlock(int16_t count, float x, float y, uint8_t type);
    void DefenseInterruptBlock(int16_t count, float x, float y, uint8_t type);
    void AddInterruptBlock(uint8_t yRowCnt, uint8_t xCnt, std::span<const uint8_t> xIdx);
    void StopComboAttack();
    void ChatMessage(std::string_view msg);

protected:
    // NetServer 인터페이스 구현
    bool ConnectProcess(ClientInfo* client) override;
    bool DisconnectProcess(ClientInfo* client) override;        
    bool PacketProcess(ClientInfo* client, std::span<const char> packet) override;

        
private:

    void ProcessPacket(const ProcessEvent& event);
    void InitializePacketProcessors();        
    void ProcessDisconnectEvent(uint8_t player_id);
    uint8_t GenerateUniqueId() { return unique_player_id_++; }

    CriticalSection critical_section_{};
    uint8_t unique_player_id_{ 1 };  // 0은 서버 자신
    Concurrency::concurrent_queue<ProcessEvent> msg_queue_{};
    PacketProcessorMap packet_processors_{};
};

template<typename PacketType> requires std::derived_from<PacketType, PacketBase>
void GameServer::BroadcastPacket(const PacketType& packet, uint8_t exclude_id)
{
    auto& playerManager = GAME_APP.GetPlayerManager();
    PlayerManager::PlayerMap players = playerManager.GetPlayers();

    for (auto it = players.begin(); it != players.end(); ++it)
    {
        const auto& player = it->second;
        if (player && player->GetId() != exclude_id)
        {
            auto packetBytes = packet.ToBytes();
            auto packet_data = std::span<const char>{ packetBytes.data(), packetBytes.size()};

            if (SendMsg(player->GetNetInfo(), packet_data) == false)
            {
            }
        }
    }
}
