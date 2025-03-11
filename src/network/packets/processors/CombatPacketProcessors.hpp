#pragma once
/*
 *
 * 설명: 블록 전투 관련 패킷 클래스 모음
 *
 */

#include "IPacketProcessor.hpp"
#include "../PacketBase.hpp"
#include "../GamePackets.hpp"
#include "../../CriticalSection.hpp"
#include "../../../core/GameApp.hpp"
#include "../../../core/manager/StateManager.hpp"
#include "../../../states/CharacterSelectState.hpp"
#include "../../../core/manager/PlayerManager.hpp"
#include "../../../states/GameState.hpp"
#include "../../../network/NetworkController.hpp"
#include "../../../game/system/BasePlayer.hpp"
#include "../../../game/system/RemotePlayer.hpp"
#include "../../../game/system/LocalPlayer.hpp"

class AttackInterruptProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& attack_packet = static_cast<const AttackInterruptPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        // 서버의 게임 상태에 공격 처리
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            if (const auto& localPlayer = gameState->GetLocalPlayer())
            {
                localPlayer->AddInterruptBlockCount(
                    attack_packet.count,
                    attack_packet.position_x,
                    attack_packet.position_y,
                    attack_packet.block_type
                );

                // 패킷 응답 생성 및 전송
                AttackResultPlayerInterruptBlocCountPacket resultPacket;
                resultPacket.player_id = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
                resultPacket.count = localPlayer->GetTotalInterruptBlockCount();
                resultPacket.attackerCount = localPlayer->GetTotalEnemyInterruptBlockCount();


                // 모든 클라이언트에 전송
                auto& playerManager = GAME_APP.GetPlayerManager();
                {
                    CriticalSection::Lock lock(playerManager.GetCriticalSection());
                    for (const auto& [_, player] : playerManager.GetPlayers()) 
                    {
                        NETWORK.SendToClient(player->GetNetInfo(), resultPacket);
                    }
                }
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::AttackInterruptBlock;
    }
};

class DefenseInterruptProcessor : public IPacketProcessor
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override {
        const auto& defense_packet = static_cast<const DefenseInterruptPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) {
            return;
        }

        auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get());
        if (!gameState) return;

        // 서버 측 방어 처리
        if (const auto& localPlayer = gameState->GetLocalPlayer()) 
        {
            localPlayer->DefenseInterruptBlockCount(
                defense_packet.count,
                defense_packet.position_x,
                defense_packet.position_y,
                defense_packet.block_type
            );

            // 응답 패킷 생성
            DefenseResultInterruptBlockCountPacket result_packet;
            result_packet.player_id = defense_packet.player_id;
            result_packet.count = localPlayer->GetTotalEnemyInterruptBlockCount();

            // 모든 클라이언트에게 전송
            auto& playerManager = GAME_APP.GetPlayerManager();
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                NETWORK.SendToClient(player->GetNetInfo(), result_packet);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override
    {
        return PacketType::DefenseInterruptBlock;
    }
};


class AddInterruptBlockProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& interrupt_packet = static_cast<const AddInterruptBlockPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) {
            return;
        }

        auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get());
        if (!gameState) return;

        // 원격 플레이어에게 방해 블록 추가
        if (const auto& remotePlayer = gameState->GetRemotePlayer()) 
        {
            std::span<const uint8_t> indices(
                interrupt_packet.x_indices.data(),
                std::min<size_t>(interrupt_packet.x_count, interrupt_packet.x_indices.size())
            );

            remotePlayer->AddInterruptBlock(interrupt_packet.y_row_count, indices);
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::AddInterruptBlock;
    }
};

class StopComboProcessor : public IPacketProcessor {
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& combo_packet = static_cast<const StopComboPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // 다른 플레이어들에게 콤보 중지 전달
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != combo_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), combo_packet);
                }
            }
        }

        // 콤보 상태 해제
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
			gameState->GetLocalPlayer()->SetComboAttackState(false);
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override {
        return PacketType::StopComboAttack;
    }
};

class LoseGameProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override {
        const auto& lose_packet = static_cast<const LoseGamePacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // 다른 플레이어들에게 게임 종료 전달
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());

            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != lose_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), lose_packet);
                }
            }
        }

        // 게임 종료 처리
        if (auto* gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            gameState->GameQuit();

            const auto& remotePlayer = gameState->GetRemotePlayer();
            const auto& localPlayer = gameState->GetLocalPlayer();
            
            if (lose_packet.player_id == remotePlayer->GetPlayerID())
            {
                remotePlayer->LoseGame(false);
                localPlayer->LoseGame(true);
            }
            else
            {
                remotePlayer->LoseGame(true);
                localPlayer->LoseGame(false);
            }            
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::LoseGame;
    }
};
