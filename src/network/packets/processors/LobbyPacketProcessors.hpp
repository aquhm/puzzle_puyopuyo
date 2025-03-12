#pragma once
/*
 *
 * 설명: 로비 처리 관련 패킷 클래스 모음
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
#include "../../../ui/EditBox.hpp"
#include "../../../utils/StringUtils.hpp"


// 로비 관련 패킷 프로세서들
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
                std::wstring wideMessage = std::format(L"새로운 사용자가 입장하였습니다.");
                std::string message = StringUtils::WideToUtf8(wideMessage);

                edit_box->InputContent(message);
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
                std::string formatted_message = std::format("[Player({})]: {}", chat_packet.player_id, chat_packet.message.data());
                edit_box->InputContent(formatted_message);
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
