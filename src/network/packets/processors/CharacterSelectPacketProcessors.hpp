#pragma once
/*
 *
 * 설명: 캐럭터 선택 상태 처리 관련 패킷 클래스 모음
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
#include "../../../game/system/BasePlayer.hpp"
#include "../../../game/system/RemotePlayer.hpp"


class ChangeCharSelectProcessor : public IPacketProcessor
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& select_packet = static_cast<const ChangeCharSelectPacket&>(packet);

        auto& state_manager = GAME_APP.GetStateManager();
        if (state_manager.GetCurrentStateID() != StateManager::StateID::CharSelect)
        {
            return;
        }

        // 다른 플레이어들에게 선택 정보 브로드캐스트
        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());

            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != select_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), select_packet);
                }
            }
        }

        if (auto charSelect = dynamic_cast<CharacterSelectState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            charSelect->SetEnemySelectPos(select_packet.x_pos, select_packet.y_pos);
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::ChangeCharSelect;
    }
};

class DecideCharacterProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& decide_packet = static_cast<const DecideCharacterPacket&>(packet);

        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::CharSelect) 
        {
            return;
        }

        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());

            if (auto player = playerManager.FindPlayer(decide_packet.player_id))
            {
                player->SetCharacterId(decide_packet.y_pos * 7 + decide_packet.x_pos);
            }

            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                if (player->GetId() != decide_packet.player_id) 
                {
                    NETWORK.SendToClient(player->GetNetInfo(), decide_packet);
                }
            }
        }

        // 캐릭터 선택 상태 업데이트
        if (auto charSelect = static_cast<CharacterSelectState*>(GAME_APP.GetStateManager().GetCurrentState().get())) 
        {
            charSelect->SetEnemyDecide(decide_packet.x_pos, decide_packet.y_pos);
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::DecideCharSelect;
    }
};
