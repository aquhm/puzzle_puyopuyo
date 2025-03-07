#pragma once
/*
 *
 * 설명: 게임 초기화 및 재시작 관련 클래스 모음
 *
 */

#include "IPacketProcessor.hpp"
#include "../GamePackets.hpp"
#include "../../CriticalSection.hpp"
#include "../../../core/GameApp.hpp"
#include "../../../core/manager/StateManager.hpp"
#include "../../../states/CharacterSelectState.hpp"
#include "../../../core/manager/PlayerManager.hpp"
#include "../../../states/GameState.hpp"
#include "../../../network/NetworkController.hpp"
#include "../../../network/player/Player.hpp"


class InitializePlayerProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& init_packet = static_cast<const InitializePlayerPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        // 플레이어 캐릭터 ID 설정
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            if (auto player = playerManager.FindPlayer(init_packet.player_id))
            {
                player->SetCharacterId(init_packet.character_idx);
            }

            // 다른 플레이어들에게 브로드캐스트
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != init_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), init_packet);
                }
            }
        }

        // 게임 플레이어 생성
        if (auto gameState = dynamic_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            std::span<const uint8_t, 2> blockType1{ init_packet.block_type1 };
            std::span<const uint8_t, 2> blockType2{ init_packet.block_type2 };

            gameState->CreateGamePlayer(blockType1, blockType2, init_packet.player_id, init_packet.character_idx);
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::InitializePlayer;
    }
};

class RestartGameProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& restart_packet = static_cast<const RestartGamePacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // 다른 플레이어들에게 브로드캐스트
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != restart_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), restart_packet);
                }
            }
        }

        // 게임 재시작
        if (auto game_state = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (const auto& remotePlayer = game_state->GetRemotePlayer())
            {
                std::span<const uint8_t, 2> blockType1{ restart_packet.block1 };
                std::span<const uint8_t, 2> blockType2{ restart_packet.block2 };

                remotePlayer->Restart(blockType1, blockType2);

                // 로그 기록
                LOGGER.Info("Game restarted by player {}", restart_packet.player_id);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::RestartGame;
    }
};

