#pragma once
/*
 *
 * ����: �⺻ ��Ŷ �������̽�
 *
 */
#include <cstdint>


class IPacket 
{
public:
    virtual ~IPacket() = default;
    virtual uint16_t GetType() const = 0;
};
