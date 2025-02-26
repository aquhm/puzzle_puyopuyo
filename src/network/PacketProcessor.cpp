#include "PacketProcessor.hpp"
#include "../utils/Logger.hpp"

void PacketProcessor::ProcessPacket(uint8_t connectionId, std::span<const char> data, uint32_t length)
{
    if (length < sizeof(PacketBase))
    {
        LOGGER.Warning("Invalid packet: too small");
        return;
    }

    const PacketBase* basePacket = reinterpret_cast<const PacketBase*>(data.data());

    // 패킷 크기 검증
    if (basePacket->size != length)
    {
        LOGGER.Warning("Packet size mismatch: expected {}, got {}",
            basePacket->size, static_cast<unsigned int>(length));
        return;
    }

    // 패킷 타입 변환 및 검증
    PacketType packetType = static_cast<PacketType>(basePacket->type);
    if (!IsValidPacketType(packetType))
    {
        LOGGER.Warning("Invalid packet type: {}", static_cast<int>(packetType));
        return;
    }

    // 핸들러 호출
    auto it = handlers_.find(packetType);
    if (it != handlers_.end())
    {
        it->second(connectionId, data);
    }
    else
    {
        LOGGER.Warning("No handler registered for packet type: {}", static_cast<int>(packetType));
    }
}

void PacketProcessor::ClearHandlers()
{
    handlers_.clear();
}