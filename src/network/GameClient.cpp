#include "GameClient.hpp"
#include "../states/GameState.hpp"
#include "./packets/GamePackets.hpp"
#include "../core/manager/StateManager.hpp"
#include "../core/GameApp.hpp"

#include <format>
#include <algorithm>
#include <SDL3/SDL_log.h>



bool GameClient::Start(HWND hwnd)
{
    if (!NetClient::Start(hwnd))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "NetClient Start failed");
        return false;
    }
    return true;
}

void GameClient::Exit()
{
    NetClient::Exit();
}

void GameClient::ProcessConnectExit()
{
    NetClient::ProcessConnectExit();

    auto& state_manager = GAME_APP.GetStateManager();
    switch (state_manager.GetCurrentStateID())
    {
    case StateManager::StateID::Room:
    case StateManager::StateID::CharSelect:
    case StateManager::StateID::Game:
        state_manager.ChangeState(StateManager::StateID::Login);
        break;
    }
}

void GameClient::ChatMessage(std::string_view msg)
{

    ChatMessagePacket packet;
    packet.player_id = player_id_;
    packet.SetMessage(msg);

    SendPacketInternal(packet);
}

void GameClient::ChangeCharSelect(uint8_t x, uint8_t y) 
{

    ChangeCharSelectPacket packet;
    packet.player_id = player_id_;
    packet.x_pos = x;
    packet.y_pos = y;

    SendPacketInternal(packet);
}

void GameClient::DecideCharacter(uint8_t x, uint8_t y)
{
    DecideCharacterPacket packet;
    packet.player_id = player_id_;
    packet.x_pos = x;
    packet.y_pos = y;

    SendPacketInternal(packet);
}

void GameClient::AddNewBlock(std::span<const uint8_t> block)
{
    AddNewBlockPacket packet;

    packet.player_id = player_id_;
    packet.block_type;  // 블록 타입 2개

    SendPacketInternal(packet);
}

void GameClient::GameInitialize(std::span<const uint8_t> block1, std::span<const uint8_t> block2) 
{

    GameInitPacket packet;
    packet.player_id = player_id_;

    if (block1.size() >= 2) {
        std::copy_n(block1.data(), 2, packet.block1.begin());
    }
    if (block2.size() >= 2) {
        std::copy_n(block2.data(), 2, packet.block2.begin());
    }

    SendPacketInternal(packet);
}

void GameClient::MoveBlock(uint8_t moveType, float position) {

    MoveBlockPacket packet;
    packet.player_id = player_id_;
    packet.move_type = moveType;
    packet.position = position;

    SendPacketInternal(packet);
}

void GameClient::RotateBlock(uint8_t rotateType, bool isHorizontalMoving) {

    RotateBlockPacket packet;
    packet.player_id = player_id_;
    packet.rotate_type = rotateType;
    packet.is_horizontal_moving = isHorizontalMoving;

    SendPacketInternal(packet);
}
    
// 공격/방어 관련 Send 함수들
void GameClient::AttackInterruptBlock(int16_t count, float x, float y, uint8_t type) {

    AttackInterruptPacket packet;
    packet.player_id = player_id_;
    packet.count = count;
    packet.position_x = x;
    packet.position_y = y;
    packet.block_type = type;

    SendPacketInternal(packet);
}

void GameClient::DefenseInterruptBlock(int16_t count, float x, float y, uint8_t type) 
{
    DefenseInterruptPacket packet;
    packet.player_id = player_id_;
    packet.count = count;
    packet.position_x = x;
    packet.position_y = y;
    packet.block_type = type;

    SendPacketInternal(packet);
}

void GameClient::AddInterruptBlock(uint8_t yRowCnt, uint8_t xCnt, std::span<const uint8_t> xIdx) {

    AddInterruptBlockPacket packet;
    packet.player_id = player_id_;
    packet.y_row_count = yRowCnt;
    packet.x_count = xCnt;

    if (xCnt > 0 && xIdx.size() >= xCnt)
    {
        std::copy_n(xIdx.data(), min(xCnt, static_cast<uint8_t>(5)), packet.x_indices.begin());
    }

    SendPacketInternal(packet);
}

// 게임 상태 관련 Send 함수들
void GameClient::CheckBlockState() {

    CheckBlockStatePacket packet;
    packet.player_id = player_id_;

    SendPacketInternal(packet);
}

void GameClient::UpdateGameBlockPos(float pos1, float pos2) {

    UpdateBlockPosPacket packet;
    packet.player_id = player_id_;
    packet.position1 = pos1;
    packet.position2 = pos2;

    SendPacketInternal(packet);
}

void GameClient::ChangeBlockState(uint8_t state) {

    ChangeBlockStatePacket packet;
    packet.player_id = player_id_;
    packet.state = state;

    SendPacketInternal(packet);
}

void GameClient::PushBlockInGame(std::span<const float> pos1, std::span<const float> pos2) 
{

    PushBlockPacket packet;
    packet.player_id = player_id_;

    if (pos1.size() >= 2) {
        std::copy_n(pos1.data(), 2, packet.position1.begin());
    }
    if (pos2.size() >= 2) {
        std::copy_n(pos2.data(), 2, packet.position2.begin());
    }

    SendPacketInternal(packet);
}

void GameClient::ChangBlockState(uint8_t state)
{
    ChangeBlockStatePacket packet;
    packet.player_id = player_id_;
    packet.state = state;

    SendPacketInternal(packet);
}

void GameClient::RequireFallingBlock(uint8_t fallingIdx, bool falling)
{
    FallingBlockPacket packet;
        
    packet.player_id = player_id_;
    packet.falling_index = fallingIdx;
    packet.is_falling = falling;

    SendPacketInternal(packet);
}

void GameClient::StopComboAttack() 
{
    StopComboPacket packet;
    packet.player_id = player_id_;

    SendPacketInternal(packet);
}

void GameClient::LoseGame() 
{
    LoseGamePacket packet;
    packet.player_id = player_id_;

    SendPacketInternal(packet);
}

void GameClient::ReStartGame(std::span<const uint8_t> block1, std::span<const uint8_t> block2) {

    RestartGamePacket packet;
    packet.player_id = player_id_;

    if (block1.size() >= 2) 
    {
        std::copy_n(block1.data(), 2, packet.block1.begin());
    }

    if (block2.size() >= 2) 
    {
        std::copy_n(block2.data(), 2, packet.block2.begin());
    }

    SendPacketInternal(packet);
}

void GameClient::ProcessPacket(std::span<const char> packet)
{
    if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
    {
        std::string_view message(packet.data(), packet.size());

        uint8_t connectionId = 0;
        uint32_t length = static_cast<uint32_t>(packet.size());
        gameState->HandleNetworkMessage(connectionId, message, length);
    }
}
