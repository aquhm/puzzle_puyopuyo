#pragma once
/*
 *
 * 설명: 기본 패킷 인터페이스
 *
 */
#include <cstdint>


class IPacket 
{
public:
    virtual ~IPacket() = default;
    virtual uint16_t GetType() const = 0;
};
