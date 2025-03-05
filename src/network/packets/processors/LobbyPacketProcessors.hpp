#pragma once
/*
 *
 * ����: �κ� ó�� ���� ��Ŷ Ŭ���� ����
 *
 */

#include "IPacketProcessor.hpp"
#include "../GamePackets.hpp"
#include "../../CriticalSection.hpp"
#include "../../../core/GameApp.hpp"
#include "../../../core/manager/StateManager.hpp"
#include "../../../states/CharacterSelectState.hpp"
#include "../../../core/manager/PlayerManager.hpp"
#include "../../../states/RoomState.hpp"
#include "../../../network/NetworkController.hpp"
#include "../../../network/player/Player.hpp"
#include "../../../game/system/GamePlayer.hpp"
#include "../../../ui/EditBox.hpp"


// �κ� ���� ��Ŷ ���μ�����
class ConnectLobbyProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override 
    {
        const auto& lobby_packet = static_cast<const ConnectLobbyPacket&>(packet);

        auto new_player = GAME_APP.GetPlayerManager().CreatePlayer(lobby_packet.id, client);

        if (new_player != nullptr) 
        {
            NotifyExistingPlayers(new_player);
            SendPlayerList(new_player);
            BroadcastNewPlayerMessage();
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    { 
        return PacketType::ConnectLobby; 
    }

private:

    void NotifyExistingPlayers(const std::shared_ptr<Player>& newPlayer)
    {
        if (!newPlayer)
        {
            return;
        }

        auto& playerManager = GAME_APP.GetPlayerManager();
        CriticalSection::Lock lock(playerManager.GetCriticalSection());

        AddPlayerPacket packet;
        packet.player_id = newPlayer->GetId();
        packet.character_id = newPlayer->GetCharacterId();

        for (const auto& [_, player] : playerManager.GetPlayers())
        {
            if (player != newPlayer && player->GetNetInfo())
            {
                NETWORK.SendToClient(player->GetNetInfo(), packet);
            }
        }
    }

    void SendPlayerList(const std::shared_ptr<Player>& newPlayer)
    {
        if (!newPlayer)
        {
            return;
        }

        auto& playerManager = GAME_APP.GetPlayerManager();
        CriticalSection::Lock lock(playerManager.GetCriticalSection());

        for (const auto& [_, player] : playerManager.GetPlayers())
        {
            if (player->GetId() != newPlayer->GetId())
            {
                AddPlayerPacket packet;
                packet.player_id = player->GetId();
                packet.character_id = player->GetCharacterId();

                NETWORK.SendToClient(newPlayer->GetNetInfo(), packet);
            }        
        }
    }   

    void BroadcastNewPlayerMessage()
    {
        if (GAME_APP.GetStateManager().GetCurrentStateID() != StateManager::StateID::Room)
        {
            return;
        }

        if (auto roomState = static_cast<RoomState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto& edit_box = roomState->GetChatBox())
            {
                edit_box->InputContent("���ο� ����ڰ� �����Ͽ����ϴ�.");
            }
        }
    }
};

class ChatMessageProcessor : public IPacketProcessor 
{
public:
    void Initialize() override {}

    void Process(const PacketBase& packet, struct ClientInfo* client) override
    {
        const auto& chat_packet = static_cast<const ChatMessagePacket&>(packet);

           
        if (auto roomState = dynamic_cast<RoomState*>(GAME_APP.GetStateManager().GetCurrentState().get()))
        {
            if (const auto edit_box = roomState->GetChatBox())
            {
                edit_box->InputContent(chat_packet.message.data());
            }
        }

        auto& playerManager = GAME_APP.GetPlayerManager();
        {
            CriticalSection::Lock lock(playerManager.GetCriticalSection());

            for (const auto& [_, player] : playerManager.GetPlayers())
            {
                NETWORK.SendToClient(player->GetNetInfo(), chat_packet);
            }
        }
    }

    void Release() override {}

    [[nodiscard]] PacketType GetPacketType() const override 
    {
        return PacketType::ChatMessage;
    }
};
