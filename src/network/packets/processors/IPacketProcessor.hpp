#pragma once
/**
 *
 * 설명: PacketBase 패킷에 대한 처리를 담당하는 추상화 인터페이스
 *
 */

#include <memory>
#include "../PacketBase.hpp"

class IPacketProcessor 
{
public:
    virtual ~IPacketProcessor() = default;
    virtual void Initialize() = 0;
    virtual void Process(const PacketBase& packet, struct ClientInfo* client) = 0;
    virtual void Release() = 0;
    [[nodiscard]] virtual PacketType GetPacketType() const = 0;
};
