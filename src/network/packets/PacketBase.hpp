#pragma once
#include "PacketType.hpp"
#include <cstdint>
#include <vector>


#pragma pack(push, 1)
struct PacketHeader 
{
    uint32_t size;      // ����� ������ ��ü ��Ŷ ũ��
    PacketType type;    // ��Ŷ Ÿ��
};

#pragma pack(pop)

class PacketBase
{
public:
    virtual ~PacketBase() = default;
    virtual PacketType GetType() const = 0;
    virtual uint32_t GetBodySize() const = 0;

    // ����� ������ ��ü ��Ŷ ũ��
    uint32_t GetTotalSize() const 
    {
        return sizeof(PacketHeader) + GetBodySize();
    }

    // ����� ������ ��ü ��Ŷ ������ ��ȯ
    std::vector<char> ToBytes() const 
    {
        // ��� �غ�
        PacketHeader header;
        header.size = GetTotalSize();
        header.type = GetType();

        // ��ü ��Ŷ ������ ����
        std::vector<char> bytes(header.size);
        std::memcpy(bytes.data(), &header, sizeof(header));
        std::memcpy(bytes.data() + sizeof(header), this, GetBodySize());

        return bytes;
    }
};