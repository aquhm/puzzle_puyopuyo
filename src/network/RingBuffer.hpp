#pragma once
/*
 *
 * 설명: 서버 Recv 링버퍼
 *
 */
#include <vector>
#include <cstring>
#include <mutex>
#include "../core/common/constants/Constants.hpp"
#include "CriticalSection.hpp"

class RingBuffer 
{ 

public:
    RingBuffer();
    ~RingBuffer() = default;

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    bool Create(size_t size = Constants::Network::MAX_RINGBUFSIZE);
    void Reset();

    inline size_t GetBufferSize() const { return buffer_size_; }
    inline char* GetBeginPos() { return begin_; }
    inline char* GetEndPos() { return end_; }
    inline char* GetCurrentBeginPos() { return current_begin_; }
    inline char* GetCurrentEndPos() { return current_end_; }
    inline char* GetProcessedPos() { return readed_; }

    char* GetBuffer(size_t requiredSize);
    char* GetBuffer(size_t movePos, size_t processedMove, size_t requireSize = Constants::Network::MAX_PACKET_SIZE);

    void ReleaseBuffer(size_t releaseSize);
    size_t GetUsedBufferSize() const { return used_size_; }
    size_t GetTotalUsedBufferSize() const { return processed_size_; }


private:

    CriticalSection critical_section_{};

    std::vector<char> buffer_;
    size_t buffer_size_;
    size_t used_size_;
    size_t processed_size_;

    char* begin_;
    char* end_;
    char* current_begin_;
    char* current_end_;
    char* readed_;
    char* recycle_;

};
