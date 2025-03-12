#include "GameServer.hpp"

#include "./packets/GamePackets.hpp"
#include "../core/manager/StateManager.hpp"
#include "../core/manager/PlayerManager.hpp"
#include "../core/GameApp.hpp"
#include "../states/RoomState.hpp"
#include "../utils/Logger.hpp"

#include "./packets/PacketType.hpp"
#include "./packets/processors/IPacketProcessor.hpp"
#include "./packets/processors/LobbyPacketProcessors.hpp"
#include "./packets/processors/CharacterSelectPacketProcessors.hpp"
#include "./packets/processors/BlockPacketProcessors.hpp"
#include "./packets/processors/CombatPacketProcessors.hpp"
#include "./packets/processors/GameInitPacketProcessors.hpp"

#include <format>
#include <unordered_map>



GameServer::GameServer()
{
    InitializePacketProcessors();
}


GameServer::~GameServer()
{
}


void GameServer::InitializePacketProcessors()
{
    // 로비 관련 프로세서
    packet_processors_.emplace(PacketType::ConnectLobby,std::make_unique<ConnectLobbyProcessor>());
    packet_processors_.emplace(PacketType::ChatMessage,std::make_unique<ChatMessageProcessor>());

    // 캐릭터 선택 관련 프로세서
    packet_processors_.emplace(PacketType::ChangeCharSelect, std::make_unique<ChangeCharSelectProcessor>());
    packet_processors_.emplace(PacketType::DecideCharSelect, std::make_unique<DecideCharacterProcessor>());

    // 블록 조작 관련 프로세서
    packet_processors_.emplace(PacketType::AddNewBlock, std::make_unique<AddNewBlockProcessor>());
    packet_processors_.emplace(PacketType::UpdateBlockFalling, std::make_unique<BlockFallingProcessor>());
    packet_processors_.emplace(PacketType::ChangeBlockState, std::make_unique<ChangeBlockStateProcessor>());
    packet_processors_.emplace(PacketType::PushBlockInGame, std::make_unique<PushBlockProcessor>());
    packet_processors_.emplace(PacketType::CheckBlockState, std::make_unique<CheckBlockStateProcessor>());
    packet_processors_.emplace(PacketType::UpdateBlockRotate, std::make_unique<BlockRotateProcessor>());
    packet_processors_.emplace(PacketType::UpdateBlockMove, std::make_unique<BlockMoveProcessor>());
    packet_processors_.emplace(PacketType::SyncBlockPositionY, std::make_unique<SyncPositionYProcessor>());

    // 전투 관련 프로세서
    packet_processors_.emplace(PacketType::AttackInterruptBlock, std::make_unique<AttackInterruptProcessor>());
    packet_processors_.emplace(PacketType::DefenseInterruptBlock, std::make_unique<DefenseInterruptProcessor>());
    packet_processors_.emplace(PacketType::AddInterruptBlock, std::make_unique<AddInterruptBlockProcessor>());
    packet_processors_.emplace(PacketType::StopComboAttack, std::make_unique<StopComboProcessor>());
    packet_processors_.emplace(PacketType::LoseGame, std::make_unique<LoseGameProcessor>());

    // 게임 초기화 관련 프로세서
    packet_processors_.emplace(PacketType::InitializePlayer, std::make_unique<InitializePlayerProcessor>());
    packet_processors_.emplace(PacketType::RestartGame, std::make_unique<RestartGameProcessor>());

    // 각 프로세서 초기화
    for (auto& [_, processor] : packet_processors_) 
    {
        processor->Initialize();
    }
}

void GameServer::Update()
{
    ProcessEvent event;
    while (msg_queue_.try_pop(event))
    {
        switch (event.event_type)
        {
        case ProcessEvent::Type::Disconnect:
            ProcessDisconnectEvent(event.player_id);
            break;

        case ProcessEvent::Type::Packet:
            if (!event.packet_data.empty()) {
                ProcessPacket(event);
            }
            break;
        }
    }
}

void GameServer::ProcessPacket(const ProcessEvent& event)
{
    if (event.packet_data.empty() || event.packet_data.size() < sizeof(PacketBase))
    {
        LOGGER.Warning("Invalid packet data");
        return;
    }

    // 패킷 기본 정보 읽기
    const PacketBase* basePacket = reinterpret_cast<const PacketBase*>(event.packet_data.data());
    PacketType packetType = static_cast<PacketType>(basePacket->type);

    // 패킷 타입 검증
    if (!IsValidPacketType(packetType))
    {
        LOGGER.Warning("Invalid packet type: {}", static_cast<int>(packetType));
        return;
    }

    // 패킷 크기 검증
    if (event.packet_data.size() != basePacket->size)
    {
        LOGGER.Warning("Invalid packet size. Expected: {}, Actual: {}",
            basePacket->size, event.packet_data.size());
        return;
    }

    // 프로세서 찾기
    auto it = packet_processors_.find(packetType);
    if (it == packet_processors_.end())
    {
        LOGGER.Warning("No processor found for packet type: {}", static_cast<int>(packetType));
        return;
    }

    // 패킷 처리
    it->second->Process(*basePacket, event.client_info);
}

bool GameServer::StartServer() 
{
    // 서버의 플레이어 생성 (ID: 1)
    unique_player_id_ = 1;

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto player = playerManager.CreatePlayer(GenerateUniqueId(), nullptr);
    if (!player) 
    {
        return false;
    }

    playerManager.SetMyPlayer(player);
    return NetServer::StartServer();
}

bool GameServer::ExitServer() 
{
    unique_player_id_ = 1;

    GAME_APP.GetPlayerManager().Release();
    return NetServer::ExitServer();
}

bool GameServer::ConnectProcess(ClientInfo* client)
{
    // 새로운 클라이언트 접속시 ID 부여
    GiveIdPacket packet;
    packet.player_id = GenerateUniqueId();

    auto packetBytes = packet.ToBytes();
    auto packet_data = std::span<const char>{ packetBytes.data(), packetBytes.size() };

    return SendMsg(client, packet_data);
}


bool GameServer::DisconnectProcess(ClientInfo* client) 
{
    if (!client) 
    {
        return false;
    }
        
    CloseSocket(client);


    uint8_t player_id = GAME_APP.GetPlayerManager().RemovePlayerInRoom(client);

    msg_queue_.push(ProcessEvent(player_id));

    return true;
}

void GameServer::ProcessDisconnectEvent(uint8_t player_id) 
{
    if (player_id == 0) 
    {
        return;
    }

    auto& stateManager = GAME_APP.GetStateManager();
    auto& playerManager = GAME_APP.GetPlayerManager();

    switch (stateManager.GetCurrentStateID())
    {
    case StateManager::StateID::Room:
    {
        if (auto roomState = dynamic_cast<RoomState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            //TODO
            /* if (auto edit_box = roomState->GetEditBox())
            {
                edit_box->InputContent(std::format(" {}님이 나가셨습니다.", player_id));
            }*/
        }
        break;
    }
    case StateManager::StateID::CharSelect:
    {
        stateManager.ChangeState(StateManager::StateID::Room);
        break;
    }
    case StateManager::StateID::Game:
    {
        if (playerManager.GetPlayerCount() == 1) 
        {
            playerManager.Release();
            ExitServer();
        }
        stateManager.ChangeState(StateManager::StateID::Login);
        break;
    }
    }
}

bool GameServer::PacketProcess(ClientInfo* client, std::span<const char> packet_data) 
{
    ProcessEvent event;
    event.packet_data = packet_data;
    event.client_info = client;

    msg_queue_.push(event);

    return true;
}

// 캐릭터 선택 관련
void GameServer::StartCharacterSelect() 
{
    // 캐릭터 선택 시작을 모든 클라이언트에게 통보        
    StartCharSelectPacket packet;
    BroadcastPacket(packet);
}

void GameServer::ChangeCharSelect(uint8_t x, uint8_t y) 
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();

    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer)
    {
        return;
    }

    // 내 캐릭터 ID 설정 (y * 7 + x 형태로 계산)
    myPlayer->SetCharacterId(y * 7 + x);

    ChangeCharSelectPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.x_pos = x;
    packet.y_pos = y;

    BroadcastPacket(packet);
}

void GameServer::DecideCharacter(uint8_t x, uint8_t y) 
{
    CriticalSection::Lock lock(critical_section_);

    auto myPlayer = GAME_APP.GetPlayerManager().GetMyPlayer();
    if (!myPlayer)
    {
        return;
    }

    // 캐릭터 선택 확정 패킷 전송
    DecideCharacterPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.x_pos = x;
    packet.y_pos = y;

    BroadcastPacket(packet);
}

// 게임 시작/초기화 관련
void GameServer::StartGame() 
{
    CriticalSection::Lock lock(critical_section_);

    // 게임 시작 패킷 전송
    StartGamePacket packet;
    BroadcastPacket(packet);
}

void GameServer::ReStartGame(std::span<const uint8_t> block1, std::span<const uint8_t> block2, uint8_t map_idx)
{
    CriticalSection::Lock lock(critical_section_);

    auto myPlayer = GAME_APP.GetPlayerManager().GetMyPlayer();
    if (!myPlayer)
    {
        return;
    }

    // 게임 재시작 패킷 전송
    RestartGamePacket packet;
    packet.player_id = myPlayer->GetId();
    packet.map_id = map_idx;

    std::copy_n(block1.begin(), packet.block1.size(), packet.block1.begin());
    std::copy_n(block2.begin(), packet.block2.size(), packet.block2.begin());

    BroadcastPacket(packet);
}

void GameServer::GameInitialize(std::span<const uint8_t> block1, std::span<const uint8_t> block2, uint8_t map_idx)
{
    CriticalSection::Lock lock(critical_section_);

    auto myPlayer = GAME_APP.GetPlayerManager().GetMyPlayer();
    if (!myPlayer)
    {
        return;
    }

    // 게임 초기화 패킷 전송
    GameInitPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.character_id = myPlayer->GetCharacterId();
    packet.map_id = map_idx;

    std::copy_n(block1.begin(), packet.block1.size(), packet.block1.begin());
    std::copy_n(block2.begin(), packet.block2.size(), packet.block2.begin());

    BroadcastPacket(packet);
}

void GameServer::LoseGame()
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    LoseGamePacket packet;
    packet.player_id = myPlayer->GetId();

    BroadcastPacket(packet);
}


// 블록 조작 관련
void GameServer::AddNewBlock(std::span<const uint8_t> block)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    AddNewBlockPacket packet;
    packet.player_id = myPlayer->GetId();

    std::copy_n(block.begin(), packet.block_type.size(), packet.block_type.begin());
        

    BroadcastPacket(packet);
}

void GameServer::MoveBlock(uint8_t moveType, float position)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    MoveBlockPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.move_type = moveType;
    packet.position = position;

    BroadcastPacket(packet);
}

void GameServer::RotateBlock(uint8_t rotateType, bool horizontalMoving)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    RotateBlockPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.rotate_type = rotateType;
    packet.is_horizontal_moving = horizontalMoving;

    BroadcastPacket(packet);
}

void GameServer::CheckBlockState()
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    CheckBlockStatePacket packet;
    packet.player_id = myPlayer->GetId();

    BroadcastPacket(packet);
}

void GameServer::UpdateGameBlockPos(float pos1, float pos2)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    UpdateBlockPosPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.position1 = pos1;
    packet.position2 = pos2;

    BroadcastPacket(packet);
}

void GameServer::RequireFallingBlock(uint8_t fallingIdx, bool falling)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    FallingBlockPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.falling_index = fallingIdx;
    packet.is_falling = falling;

    BroadcastPacket(packet);
}

void GameServer::ChangBlockState(uint8_t state)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    ChangeBlockStatePacket packet;
    packet.player_id = myPlayer->GetId();
    packet.state = state;

    BroadcastPacket(packet);
}

void GameServer::PushBlockInGame(std::span<const float> pos1, std::span<const float> pos2)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    PushBlockPacket packet;
    packet.player_id = myPlayer->GetId();

    std::copy_n(pos1.begin(), packet.position1.size(), packet.position1.begin());
    std::copy_n(pos2.begin(), packet.position2.size(), packet.position2.begin());

    BroadcastPacket(packet);
}

void GameServer::SyncPositionY(float positionY, float velocity)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    SyncBlockPositionYPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.position_y = positionY;
    packet.velocity = velocity;

    BroadcastPacket(packet);
}


// 전투/인터럽트 관련

void GameServer::AttackInterruptBlock(int16_t count, float x, float y, uint8_t type)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    AttackInterruptPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.count = count;
    packet.position_x = x;
    packet.position_y = y;
    packet.block_type = type;

    BroadcastPacket(packet);
}

void GameServer::DefenseInterruptBlock(int16_t count, float x, float y, uint8_t type)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    DefenseInterruptPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.count = count;
    packet.position_x = x;
    packet.position_y = y;
    packet.block_type = type;

    BroadcastPacket(packet);
}

void GameServer::AddInterruptBlock(uint8_t yRowCnt, uint8_t xCnt, std::span<const uint8_t> xIdx)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    AddInterruptBlockPacket packet;
    packet.player_id = myPlayer->GetId();
    packet.y_row_count = yRowCnt;
    packet.x_count = xCnt;

    if (xCnt > 0) 
    {
        std::copy_n(xIdx.begin(), packet.x_indices.size(), packet.x_indices.begin());
    }

    BroadcastPacket(packet);
}

void GameServer::StopComboAttack()
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (!myPlayer) {
        return;
    }

    StopComboPacket packet;
    packet.player_id = myPlayer->GetId();

    BroadcastPacket(packet);
}

void GameServer::ChatMessage(std::string_view msg)
{
    CriticalSection::Lock lock(critical_section_);

    auto& playerManager = GAME_APP.GetPlayerManager();
    auto myPlayer = playerManager.GetMyPlayer();
    if (myPlayer == nullptr) 
    {
        return;
    }

    ChatMessagePacket packet;
    packet.player_id = myPlayer->GetId();
    packet.SetMessage(msg);

    BroadcastPacket(packet);
}


