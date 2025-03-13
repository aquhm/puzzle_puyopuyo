#include "NetworkController.hpp"
#include "GameServer.hpp"
#include "GameClient.hpp"

#include "../core/GameApp.hpp"
#include "../core/manager/StateManager.hpp"

#include "../states/GameState.hpp"
#include "../game/map/GameBackground.hpp"

#include "./packets/PacketBase.hpp"

NetworkController& NetworkController::Instance() 
{
    static NetworkController instance;
    return instance;
}

void NetworkController::Initialize(NetworkRole role) 
{
    role_ = role;
    is_running_ = true;

    if (role_ == NetworkRole::Server) 
    {
        server_ = std::make_unique<GameServer>();
    }
    else if (role_ == NetworkRole::Client) 
    {
        client_ = std::make_unique<GameClient>();
    }
}

void NetworkController::Shutdown() 
{
    Stop();

    server_.reset();
    client_.reset();

    role_ = NetworkRole::None;
    is_running_ = false;
}

bool NetworkController::Start() 
{
    if (role_ == NetworkRole::Server) 
    {
        return server_ && server_->StartServer();
    }
    else if (role_ == NetworkRole::Client) 
    {
        return client_ && client_->Start(GAME_APP.GetWindowHandle());
    }
    return false;
}

void NetworkController::Stop() 
{
    if (role_ == NetworkRole::Server) 
    {
        if (server_) server_->ExitServer();
    }
    else if (role_ == NetworkRole::Client) 
    {
        if (client_) client_->Exit();
    }
}

void NetworkController::SendData(std::span<const char> data)
{
    if (role_ == NetworkRole::Client)
    {
        if (client_)
        {
            client_->SendData(data);
        }
    }
}

bool NetworkController::SendToClient(ClientInfo* client, const PacketBase& packet)
{
    if (!IsServer() || !server_ || !client)
        return false;

    auto packetBytes = packet.ToBytes();
    auto packet_data = std::span<const char>{ packetBytes.data(), packetBytes.size() };

    return server_->SendMsg(client, packet_data);
}


void NetworkController::Update() 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->Update();
    }
}

// 게임 상태 제어 함수들 구현
void NetworkController::GameInitialize(std::span<const uint8_t> block1, std::span<const uint8_t> block2) 
{
        
    if (role_ == NetworkRole::Server && server_) 
    {
        if (auto* game_state = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (auto* background = game_state->GetBackGround()) 
            {
                server_->GameInitialize(block1, block2, background->GetMapIndex());
            }
        }
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->GameInitialize(block1, block2);
    }
}

void NetworkController::StartCharacterSelect() 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->StartCharacterSelect();
    }
}

void NetworkController::ChangeCharSelect(uint8_t x, uint8_t y) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->ChangeCharSelect(x, y);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->ChangeCharSelect(x, y);
    }
}

void NetworkController::DecideCharacter(uint8_t x, uint8_t y) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->DecideCharacter(x, y);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->DecideCharacter(x, y);
    }
}

// 블록 조작 관련
void NetworkController::AddNewBlock(std::span<const uint8_t> block) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->AddNewBlock(block);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->AddNewBlock(block);
    }
}

void NetworkController::MoveBlock(uint8_t move_type, float position) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->MoveBlock(move_type, position);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->MoveBlock(move_type, position);
    }
}

void NetworkController::RotateBlock(uint8_t rotate_type, bool horizontal_moving) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->RotateBlock(rotate_type, horizontal_moving);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->RotateBlock(rotate_type, horizontal_moving);
    }
}

// 전투/인터럽트 관련
void NetworkController::AttackInterruptBlock(int16_t count, float x, float y, uint8_t type) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->AttackInterruptBlock(count, x, y, type);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->AttackInterruptBlock(count, x, y, type);
    }
}

void NetworkController::DefenseInterruptBlock(int16_t count, float x, float y, uint8_t type) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->DefenseInterruptBlock(count, x, y, type);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->DefenseInterruptBlock(count, x, y, type);
    }
}

void NetworkController::AddInterruptBlock(uint8_t y_row_count, uint8_t x_count, std::span<const uint8_t> xIdx) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->AddInterruptBlock(y_row_count, x_count, xIdx);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->AddInterruptBlock(y_row_count, x_count, xIdx);
    }
}

// 게임 상태 제어
void NetworkController::ReStartGame(std::span<const uint8_t> block1, std::span<const uint8_t> block2) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        if (auto* game_state = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (auto* background = game_state->GetBackGround()) 
            {
                server_->ReStartGame(block1, block2, background->GetMapIndex());
            }
        }
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->ReStartGame(block1, block2);
    }
}

void NetworkController::ChatMessage(std::string_view msg) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->ChatMessage(msg);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->ChatMessage(msg);
    }
}

void NetworkController::StartGame() 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->StartGame();
    }
}

void NetworkController::CheckBlockState() 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->CheckBlockState();
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->CheckBlockState();
    }
}

void NetworkController::UpdateGameBlockPos(float pos1, float pos2) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->UpdateGameBlockPos(pos1, pos2);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->UpdateGameBlockPos(pos1, pos2);
    }
}

void NetworkController::RequireFallingBlock(uint8_t falling_idx, bool falling) 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->RequireFallingBlock(falling_idx, falling);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->RequireFallingBlock(falling_idx, falling);
    }
}

void NetworkController::ChangeBlockState(uint8_t state)
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->ChangBlockState(state);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->ChangBlockState(state);
    }
}

void NetworkController::PushBlockInGame(std::span<const float> pos1, std::span<const float> pos2)
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->PushBlockInGame(pos1, pos2);
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->PushBlockInGame(pos1, pos2);
    }
}

void NetworkController::SyncPositionY(float positionY, float velocity)
{
    if (role_ == NetworkRole::Server && server_)
    {
        server_->SyncPositionY(positionY, velocity);
    }
    else if (role_ == NetworkRole::Client && client_)
    {
        client_->SyncPositionY(positionY, velocity);
    }
}

void NetworkController::StopComboAttack() 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->StopComboAttack();
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->StopComboAttack();
    }
}

void NetworkController::LoseGame() 
{
    if (role_ == NetworkRole::Server && server_) 
    {
        server_->LoseGame();
    }
    else if (role_ == NetworkRole::Client && client_) 
    {
        client_->LoseGame();
    }
}

bool NetworkController::ProcessRecv(WPARAM wParam, LPARAM lParam)
{
     if (role_ == NetworkRole::Client && client_)
     {
         return client_->ProcessRecv(wParam, lParam);
     }
     return false;
}


