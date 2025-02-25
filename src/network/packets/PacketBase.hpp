#pragma once
#include "PacketType.hpp"
#include <cstdint>
#include <vector>


#pragma pack(push, 1)
struct PacketHeader 
{
    uint32_t size;      // 헤더를 포함한 전체 패킷 크기
    PacketType type;    // 패킷 타입
};

#pragma pack(pop)

class PacketBase
{
public:
    virtual ~PacketBase() = default;
    virtual PacketType GetType() const = 0;
    virtual uint32_t GetBodySize() const = 0;

    // 헤더를 포함한 전체 패킷 크기
    uint32_t GetTotalSize() const 
    {
        return sizeof(PacketHeader) + GetBodySize();
    }

    // 헤더를 포함한 전체 패킷 데이터 반환
    std::vector<char> ToBytes() const 
    {
        // 헤더 준비
        PacketHeader header;
        header.size = GetTotalSize();
        header.type = GetType();

        // 전체 패킷 데이터 생성
        std::vector<char> bytes(header.size);
        std::memcpy(bytes.data(), &header, sizeof(header));
        std::memcpy(bytes.data() + sizeof(header), this, GetBodySize());

        return bytes;
    }
};