#pragma once
#include "PacketType.hpp"
#include <cstdint>
#include <vector>


#pragma pack(push, 1)
/*struct PacketHeader 
{
    uint32_t size;
    PacketType type;
};*/
#pragma pack(pop)

// 모든 패킷의 기본 클래스
class PacketBase 
{
public:
    virtual ~PacketBase() = default;
    virtual PacketType GetType() const = 0;
    virtual uint32_t GetSize() const = 0;

    virtual std::vector<char> ToBytes() const 
    {
        std::vector<char> bytes(sizeof(*this));
        std::memcpy(bytes.data(), this, sizeof(*this));
        return bytes;
    }
};

