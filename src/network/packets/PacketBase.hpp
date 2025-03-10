#pragma once
#include "PacketType.hpp"
#include <cstdint>
#include <vector>
#include <string_view>

#pragma pack(push, 1)
struct PacketBase
{
    uint32_t size;      // 패킷 전체 크기
    uint16_t type;      // 패킷 타입 (PacketType을 uint16_t로 처리)

    // 패킷 데이터를 바이트 배열로 변환
    std::vector<char> ToBytes() const
    {
        std::vector<char> bytes(size);
        std::memcpy(bytes.data(), this, size);
        return bytes;
    }
};

#pragma pack(pop)