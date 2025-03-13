#pragma once
/*
 *
 * 설명: Block 처리 관련 패킷 클래스 모음
 *
 */

#include "IPacketProcessor.hpp"
#include "../GamePackets.hpp"
#include "../../../core/GameApp.hpp"
#include "../../../core/manager/StateManager.hpp"
#include "../../../core/manager/PlayerManager.hpp"
#include "../../../states/GameState.hpp"
#include "../../../network/NetworkController.hpp"
#include "../../../game/system/BasePlayer.hpp"
#include "../../../game/system/RemotePlayer.hpp"

#include <span>


class AddNewBlockProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& block_packet = static_cast<const AddNewBlockPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != block_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), block_packet);
                }
            }
        }

        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {   
                std::span<const uint8_t, 2> blockType{ block_packet.block_type };
                remotePlayer->AddNewBlock(blockType);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::AddNewBlock;
    }
};

class BlockFallingProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& fall_packet = static_cast<const FallingBlockPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != fall_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), fall_packet);
                }
            }
        }

        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->UpdateFallingBlock(fall_packet.falling_index, fall_packet.is_falling);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::UpdateBlockFalling;
    }
};

class ChangeBlockStateProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& state_packet = static_cast<const ChangeBlockStatePacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // 다른 플레이어들에게 상태 변경 브로드캐스트
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != state_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), state_packet);
                }
            }
        }

        // 게임 상태 업데이트
        if (auto gameState = static_cast<GameState*>( GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->ChangeBlockState(state_packet.state);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::ChangeBlockState;
    }
};

class PushBlockProcessor : public IPacketProcessor
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& pushPacket = static_cast<const PushBlockPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        // 다른 플레이어들에게 블록 푸시 브로드캐스트
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != pushPacket.player_id)
                {
                    NETWORK.SendToClient(player->GetNetInfo(), pushPacket);
                }
            }
        }

        // 게임 상태 업데이트
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                float pos1[2] = { pushPacket.position1[0], pushPacket.position1[1] };
                float pos2[2] = { pushPacket.position2[0], pushPacket.position2[1] };
                remotePlayer->PushBlockInGame(pos1, pos2);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::PushBlockInGame;
    }
};

class CheckBlockStateProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& check_packet = static_cast<const CheckBlockStatePacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        // 다른 플레이어들에게 체크 상태 전달
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != check_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), check_packet);
                }
            }
        }

        // 블록 상태 체크 수행
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->CheckGameBlockState();
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::CheckBlockState;
    }
};

class BlockRotateProcessor : public IPacketProcessor {
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& rotate_packet = static_cast<const RotateBlockPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // 다른 플레이어들에게 회전 정보 전달
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != rotate_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), rotate_packet);
                }
            }
        }

        // 게임 상태 업데이트
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->RotateBlock(rotate_packet.rotate_type, rotate_packet.is_horizontal_moving);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override {
        return PacketType::UpdateBlockRotate;
    }
};

//class ComboProcessor : public IPacketProcessor 
//{
//public:
//    void Initialize() override {}

//    void Process(const PacketBase& packet, struct ClientInfo* client) override 
//    {
//        const auto& combo_packet = static_cast<const ComboPacket&>(packet);

//        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) {
//            return;
//        }

//        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
//        {
//            gameState->ProcessCombo(combo_packet.combo_count);

//            // 다른 플레이어들에게 콤보 정보 전달
//            auto& player_manager = GAME_APP.GetPlayerManager();
//            {
//                CriticalSection::Lock lock(playerManager.GetCriticalSection());

//                for (const auto& [_, player] : player_manager.GetPlayers()) 
//                {
//                    if (player->GetId() != combo_packet.player_id) 
//                    {
//                        NETWORK.SendToClient(player->GetNetInfo(), combo_packet);
//                    }
//                }
//            }
//        }
//    }

//    void Release() override {}

//    [[nodiscard]] PacketType GetPacketType() const override 
//    {
//        return PacketType::ComboUpdate;
//    }
//};


class BlockFallProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& fall_packet = static_cast<const FallingBlockPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // 다른 플레이어들에게 블록 낙하 정보 전달
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());

            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != fall_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), fall_packet);
                }
            }
        }

        // 게임 상태 업데이트
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->UpdateFallingBlock(fall_packet.falling_index, fall_packet.is_falling);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override {
        return PacketType::UpdateBlockFalling;
    }
};

class BlockMoveProcessor : public IPacketProcessor
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override {
        const auto& move_packet = static_cast<const MoveBlockPacket&>(packet);

        // 다른 플레이어들에게 브로드캐스트
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != move_packet.player_id)
                {
                    NETWORK.SendToClient(player->GetNetInfo(), move_packet);
                }
            }
        }

        // 게임 상태 업데이트
        if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->MoveBlock(move_packet.move_type, move_packet.position);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override
    {
        return PacketType::UpdateBlockMove;
    }
};

class SyncPositionYProcessor : public IPacketProcessor
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override
    {
        const auto& sync_packet = static_cast<const SyncBlockPositionYPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        // 모든 플레이어에게 전달
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != sync_packet.player_id)
                {
                    NETWORK.SendToClient(player->GetNetInfo(), sync_packet);
                }
            }
        }

        // 게임 상태 업데이트
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto& remotePlayer = gameState->GetRemotePlayer())
            {
                remotePlayer->SyncPositionY(sync_packet.position_y, sync_packet.velocity);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override
    {
        return PacketType::SyncBlockPositionY;
    }
};