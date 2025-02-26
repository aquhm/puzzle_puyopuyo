#include "PacketProcessor.hpp"
#include "../utils/Logger.hpp"

void PacketProcessor::ProcessPacket(uint8_t connectionId, std::span<const char> data, uint32_t length) 
{
    if (length < sizeof(PacketHeader)) 
    {
        LOGGER.Warning("Invalid packet: too small");
        return;
    }

    PacketHeader header;
    std::memcpy(&header, data.data(), sizeof(PacketHeader));

    if (!IsValidPacketType(header.type)) 
    {
        LOGGER.Warning("Invalid packet type: {}", static_cast<int>(header.type));
        return;
    }

    if (header.size != length) 
    {
        LOGGER.Warning("Packet size mismatch: expected {}, got {}", header.size, static_cast<unsigned int>(length));
        return;
    }

    auto it = handlers_.find(header.type);
    if (it != handlers_.end()) 
    {
        it->second(connectionId, data);
    }
    else 
    {
        LOGGER.Warning("No handler registered for packet type: {}", static_cast<int>(header.type));
    }
}

void PacketProcessor::ClearHandlers() 
{
    handlers_.clear();
}