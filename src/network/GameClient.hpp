#pragma once
/*
 *
 * ����: Ŭ���̾�Ʈ ��Ŷ ó�� WSAEventSelect
 *
 */

#include "NetClient.hpp"
#include "CriticalSection.hpp"
#include "../network/packets/PacketBase.hpp"
#include "../network/packets/GamePackets.hpp"

#include <concepts>


class GameClient final : public NetClient 
{
public:
    GameClient() = default;
    ~GameClient() override = default;

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;
    GameClient(GameClient&&) = delete;
    GameClient& operator=(GameClient&&) = delete;

    bool Start(HWND hwnd) override;
    void Exit() override;

    // ���� ���� ��Ŷ ���� �Լ���
    void ChatMessage(std::string_view msg);
    void ChangeCharSelect(uint8_t x, uint8_t y);
    void DecideCharacter(uint8_t x, uint8_t y);
    void GameInitialize(std::span<const uint8_t> block1, std::span<const uint8_t> block2);
    void AddNewBlock(std::span<const uint8_t> block);
    void MoveBlock(uint8_t moveType, float position);
    void RotateBlock(uint8_t rotateType, bool isHorizontalMoving);
    void CheckBlockState();
    void UpdateGameBlockPos(float pos1, float pos2);
    void RequireFallingBlock(uint8_t fallingIdx, bool isFalling);
    void ChangeBlockState(uint8_t state);
    void PushBlockInGame(std::span<const float> pos1, std::span<const float> pos2);
    void ChangBlockState(uint8_t state);

    // ����/��� ����
    void AttackInterruptBlock(int16_t count, float x, float y, uint8_t type);
    void DefenseInterruptBlock(int16_t count, float x, float y, uint8_t type);
    void AddInterruptBlock(uint8_t yRowCnt, uint8_t xCnt, std::span<const uint8_t> xIdx);
    void StopComboAttack();
    void LoseGame();
    void ReStartGame(std::span<const uint8_t> block1, std::span<const uint8_t> block2);

protected:
    void ProcessPacket(std::span<const char> packet) override;
    void ProcessConnectExit() override;

private:
    template<typename PacketType> requires std::derived_from<PacketType, PacketBase>
    void SendPacketInternal(const PacketType& packet)
    {
        SendData(std::span<const char>{reinterpret_cast<const char*>(&packet), packet.GetSize()});
    }

    CriticalSection critical_section_{};
    uint8_t player_id_{ 0 };
};
