#include "PlayerManager.hpp"

#include "../../core/GameApp.hpp"
#include "../../network/player/Player.hpp"
#include "../../network/NetworkController.hpp"
#include "../../network/packets/GamePackets.hpp"
#include "../../utils/Logger.hpp"


bool PlayerManager::Initialize() 
{
    try 
    {
        players_.reserve(16);
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "PlayerManager 초기화 실패: %s", e.what());
        return false;
    }
}

void PlayerManager::Update(float deltaTime) 
{
}

void PlayerManager::Release() 
{
    CriticalSection::Lock lock(critical_section_);
    players_.clear();
    my_player_.reset();
}

std::shared_ptr<Player> PlayerManager::CreatePlayer(uint8_t id, ClientInfo* net_info) 
{
    CriticalSection::Lock lock(critical_section_);

    if (players_.contains(id)) 
    {
        return nullptr;
    }

    auto player = std::make_shared<Player>();
    player->Initialize();
    player->SetId(id);
    player->SetNetInfo(net_info);    

    players_[id] = player;
    return player;
}

uint8_t PlayerManager::RemovePlayerByNetInfo(ClientInfo* net_info) 
{
    CriticalSection::Lock lock(critical_section_);

    auto it = std::next(players_.begin());
    while (it != players_.end()) 
    {
        if (it->second->GetNetInfo() == net_info) 
        {
            uint8_t removed_id = it->first;
            it = players_.erase(it);
            return removed_id;
        }
        ++it;
    }
    return 0;
}

std::shared_ptr<Player> PlayerManager::FindPlayer(uint8_t id) 
{
    CriticalSection::Lock lock(critical_section_);
    auto it = players_.find(id);
    return (it != players_.end()) ? it->second : nullptr;
}

bool PlayerManager::RemovePlayer(uint8_t id)
{
    CriticalSection::Lock lock(critical_section_);

    auto it = players_.find(id);
    if (it != players_.end())
    {
        players_.erase(it);
        return true;
    }

    return false;
}

uint8_t PlayerManager::RemovePlayerInRoom(ClientInfo* pNetInfo) 
{
    CriticalSection::Lock lock(critical_section_);

    auto iter = std::next(players_.begin());

    while (iter != players_.end()) 
    {
        if (iter->second->GetNetInfo() == pNetInfo) 
        {
            RemovePlayerPacket packet;
            packet.player_id = iter->second->GetId();

            uint8_t removed_id = packet.player_id;
            iter = players_.erase(iter);

            if (players_.size() > 1) 
            {
                for (const auto& [_, player] : players_) 
                {
                    if (player->GetNetInfo()) 
                    {
                        NETWORK.SendData(packet);
                    }
                }
            }
            return removed_id;
        }
        ++iter;
    }
    return 0;
}

bool PlayerManager::IsLocalPlayer(uint8_t playerId) 
{ 
    return  my_player_ != nullptr && my_player_->GetId() == playerId;
}

bool PlayerManager::IsRemotePlayer(uint8_t playerId) 
{ 
    return my_player_ != nullptr && my_player_->GetId() != playerId;
}
