#pragma once

#include <functional>
#include <span>
#include <unordered_map>
#include <concepts>

#include "packets/PacketType.hpp"
#include "packets/PacketBase.hpp"

class PacketProcessor 
{
public:
    PacketProcessor() = default;
    ~PacketProcessor() = default;

    template<std::derived_from<PacketBase> T>
    void RegisterHandler(PacketType type, std::function<void(uint8_t, const T*)> handler);

    void ProcessPacket(uint8_t connectionId, std::span<const char> data, uint32_t length);

    void ClearHandlers();

private:
    
    std::unordered_map<PacketType, std::function<void(uint8_t, std::span<const char>)>> handlers_;
};

template<std::derived_from<PacketBase> T>
void PacketProcessor::RegisterHandler(PacketType type, std::function<void(uint8_t, const T*)> handler) 
{
    handlers_[type] = [handler](uint8_t connectionId, std::span<const char> data) 
        {
            if (data.size() < sizeof(T)) 
            {
                return;
            }

            const T* packet = reinterpret_cast<const T*>(data.data());
            handler(connectionId, packet);
        };
}