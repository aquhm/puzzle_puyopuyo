#pragma once
/*
 *
 * ����: ��� ���� ���� ��Ŷ Ŭ���� ����
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
#include "../../../network/player/Player.hpp"
#include "../../../game/system/GamePlayer.hpp"

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

        // ������ ���� ���¿� ���� ó��
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {

            // Ŭ�� -> ���� ���� �߻�ü ����
            gameState->AddInterruptBlockCount(
                attack_packet.count,
                attack_packet.position_x,
                attack_packet.position_y,
                attack_packet.block_type
            );

            // ������ ���ݴ��� ���ŵ� ��� ������ Ŭ��鿡�� ����
            AttackResultPlayerInterruptBlocCountPacket resultPacket;
            resultPacket.id = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
            resultPacket.count = gameState->GetTotalInterruptBlockCount();
            resultPacket.attackerCount = gameState->GetTotalInterruptEnemyBlockCount();

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
        const auto& Defense_packet = static_cast<const DefenseInterruptPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game)
        {
            return;
        }

        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {

            // Ŭ�󿡼� ������ Ŭ�� ������ ���� ����
            gameState->DefenseInterruptBlockCount(Defense_packet.count, Defense_packet.position_x, Defense_packet.position_y, Defense_packet.block_type);

            // Ŭ�� ���غ�� ���ŵ� ��� ���� �ٽ� Ŭ��� ����
            DefenseResultInterruptBlockCountPacket result_packet;
            result_packet.id = Defense_packet.player_id;
            result_packet.count = gameState->GetTotalInterruptEnemyBlockCount();

            auto& playerManager = GAME_APP.GetPlayerManager();
            {
                CriticalSection::Lock lock(playerManager.GetCriticalSection());
                for (const auto& [_, player] : playerManager.GetPlayers())
                {
                    NETWORK.SendToClient(player->GetNetInfo(), result_packet);
                }
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

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Game) 
        {
            return;
        }

        // �ٸ� �÷��̾�鿡�� ����
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());
            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != interrupt_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(),interrupt_packet);
                }
            }
        }

        // ���� ���� ������Ʈ
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {

            // ���� ��� ī��Ʈ ������Ʈ
            short current_enemy_block_count = gameState->GetTotalInterruptEnemyBlockCount();
            short total_count = interrupt_packet.y_row_count * 6 + interrupt_packet.x_count;
            current_enemy_block_count -= total_count;

            if (current_enemy_block_count < 0) 
            {
                current_enemy_block_count = 0;
            }

            gameState->SetTotalInterruptEnemyBlockCount(current_enemy_block_count);

            // ��� �߰�
            if (const auto& player = gameState->GetPlayer()) 
            {
                player->AddInterruptBlock(current_enemy_block_count);
            }
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

        // �ٸ� �÷��̾�鿡�� �޺� ���� ����
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

        // �޺� ���� ����
        if (auto gameState = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            gameState->SetAttackComboState(false);
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

        // �ٸ� �÷��̾�鿡�� ���� ���� ����
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

        // ���� ���� ó��
        if (auto* game_state = static_cast<GameState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            game_state->GameQuit();

            if (const auto& player = game_state->GetPlayer()) 
            {
                player->LoseGame(false);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::LoseGame;
    }
};
