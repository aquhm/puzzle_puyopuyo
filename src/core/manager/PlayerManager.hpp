#pragma once
/*
 *
 * 설명: 퍼즐 게임에 접속한 Player 관리 Class
 *
 */

#include "../../network/CriticalSection.hpp"
#include "../../core/manager/IManager.hpp"

#include <unordered_map>
#include <memory>

class Player;

class PlayerManager : public IManager 
{
public:

    using PlayerMap = std::unordered_map<uint8_t, std::shared_ptr<Player>>;

    PlayerManager() = default;
    ~PlayerManager() override = default;
    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;
    PlayerManager(PlayerManager&&) = delete;
    PlayerManager& operator=(PlayerManager&&) = delete;


    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    [[nodiscard]] std::string_view GetName() const override { return "PlayerManager"; }


    [[nodiscard]] std::shared_ptr<Player> CreatePlayer(uint8_t id, struct ClientInfo* net_info = nullptr);        
    [[nodiscard]] uint8_t RemovePlayerByNetInfo(struct ClientInfo* net_info);
    [[nodiscard]] std::shared_ptr<Player> FindPlayer(uint8_t id);
    [[nodiscard]] size_t GetPlayerCount() const { return players_.size(); }
    bool RemovePlayer(uint8_t id);
    uint8_t RemovePlayerInRoom(ClientInfo* pNetInfo);
    void SetMyPlayer(std::shared_ptr<Player> player) { my_player_ = player; }
    [[nodiscard]] const std::shared_ptr<Player>& GetMyPlayer() const { return my_player_; }
    [[nodiscard]] CriticalSection& GetCriticalSection() { return critical_section_; }
    [[nodiscard]] PlayerMap& GetPlayers() { return players_; }
    [[nodiscard]] bool IsLocalPlayer(uint8_t playerId);
    [[nodiscard]] bool IsRemotePlayer(uint8_t playerId);


private:

    PlayerMap players_;
    std::shared_ptr<Player> my_player_;
    CriticalSection critical_section_{};
};


