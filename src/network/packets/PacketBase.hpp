//#pragma once
//#include "PacketType.hpp"
//#include <cstdint>
//#include <vector>
//
//
//#pragma pack(push, 1)
//struct PacketHeader 
//{
//    uint32_t size;      // 헤더를 포함한 전체 패킷 크기
//    PacketType type;    // 패킷 타입
//};
//#pragma pack(pop)
//
//
//// 패킷 관련 유틸리티 함수들
//namespace PacketUtils {
//    inline std::vector<char> ToBytes(const void* packet, uint32_t size) {
//        std::vector<char> bytes(size);
//        std::memcpy(bytes.data(), packet, size);
//        return bytes;
//    }
//}
//
//class PacketBase
//{
//public:
//    virtual ~PacketBase() = default;
//    virtual PacketType GetType() const = 0;
//    virtual uint32_t GetBodySize() const = 0;
//
//    // 헤더를 포함한 전체 패킷 크기
//    uint32_t GetTotalSize() const 
//    {
//        return sizeof(PacketHeader) + GetBodySize();
//    }
//
//    // 헤더를 포함한 전체 패킷 데이터 반환
//    std::vector<char> ToBytes() const 
//    {
//        //// 헤더 준비
//        //PacketHeader header;
//        //header.size = GetTotalSize();
//        //header.type = GetType();
//
//        //// 전체 패킷 데이터 생성
//        //std::vector<char> bytes(header.size);
//        //std::memcpy(bytes.data(), &header, sizeof(header));
//        //std::memcpy(bytes.data() + sizeof(header), this, GetBodySize());
//
//        // 헤더 준비
//        PacketHeader header;
//        header.size = sizeof(PacketHeader) + GetBodySize() - sizeof(PacketBase);
//        header.type = GetType();
//
//        // 전체 패킷 데이터 생성
//        std::vector<char> bytes(header.size);
//        std::memcpy(bytes.data(), &header, sizeof(header));
//
//        // 실제 데이터 필드의 시작 위치 계산 (vptr 건너뛰기)
//        const char* data_start = reinterpret_cast<const char*>(this) + sizeof(PacketBase);
//        std::memcpy(bytes.data() + sizeof(header), data_start, GetBodySize() - sizeof(PacketBase));
//
//        return bytes;
//    }
//};

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