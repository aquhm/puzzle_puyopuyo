//#pragma once
//#include "PacketType.hpp"
//#include <cstdint>
//#include <vector>
//
//
//#pragma pack(push, 1)
//struct PacketHeader 
//{
//    uint32_t size;      // ����� ������ ��ü ��Ŷ ũ��
//    PacketType type;    // ��Ŷ Ÿ��
//};
//#pragma pack(pop)
//
//
//// ��Ŷ ���� ��ƿ��Ƽ �Լ���
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
//    // ����� ������ ��ü ��Ŷ ũ��
//    uint32_t GetTotalSize() const 
//    {
//        return sizeof(PacketHeader) + GetBodySize();
//    }
//
//    // ����� ������ ��ü ��Ŷ ������ ��ȯ
//    std::vector<char> ToBytes() const 
//    {
//        //// ��� �غ�
//        //PacketHeader header;
//        //header.size = GetTotalSize();
//        //header.type = GetType();
//
//        //// ��ü ��Ŷ ������ ����
//        //std::vector<char> bytes(header.size);
//        //std::memcpy(bytes.data(), &header, sizeof(header));
//        //std::memcpy(bytes.data() + sizeof(header), this, GetBodySize());
//
//        // ��� �غ�
//        PacketHeader header;
//        header.size = sizeof(PacketHeader) + GetBodySize() - sizeof(PacketBase);
//        header.type = GetType();
//
//        // ��ü ��Ŷ ������ ����
//        std::vector<char> bytes(header.size);
//        std::memcpy(bytes.data(), &header, sizeof(header));
//
//        // ���� ������ �ʵ��� ���� ��ġ ��� (vptr �ǳʶٱ�)
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
    uint32_t size;      // ��Ŷ ��ü ũ��
    uint16_t type;      // ��Ŷ Ÿ�� (PacketType�� uint16_t�� ó��)

    // ��Ŷ �����͸� ����Ʈ �迭�� ��ȯ
    std::vector<char> ToBytes() const
    {
        std::vector<char> bytes(size);
        std::memcpy(bytes.data(), this, size);
        return bytes;
    }
};

#pragma pack(pop)