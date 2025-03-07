#pragma once
/*
 *
 * 설명: 퍼즐 게임에 접속한 Player 정보 Class
 *
 */

#include <cstdint>

struct ClientInfo;

class Player 
{
public:
    Player() = default;
    ~Player() = default;

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&&) noexcept = default;
    Player& operator=(Player&&) noexcept = default;

    void SetId(uint8_t id) noexcept { id_ = id; }
    [[nodiscard]] uint8_t GetId() const noexcept { return id_; }

    void SetCharacterId(uint16_t id) noexcept { character_id_ = id; }
    [[nodiscard]] uint16_t GetCharacterId() const noexcept { return character_id_; }

    void SetNetInfo(ClientInfo* net_info) noexcept { net_info_ = net_info; }
    [[nodiscard]] ClientInfo* GetNetInfo() const noexcept { return net_info_; }

    void Initialize();
    void Release();

private:
    uint8_t id_{ 0 };
    uint16_t character_id_{ 0 };
    ClientInfo* net_info_{ nullptr };
};

