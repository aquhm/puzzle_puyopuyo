#include "RingBuffer.hpp"

RingBuffer::RingBuffer() :
    buffer_size_(0),
    used_size_(0),
    processed_size_(0),
    begin_(nullptr),
    end_(nullptr),
    current_begin_(nullptr),
    current_end_(nullptr),
    readed_(nullptr),
    recycle_(nullptr)
{
}

bool RingBuffer::Create(size_t size) 
{
    buffer_.clear();
    buffer_.resize(size, 0);
    std::fill(buffer_.begin(), buffer_.end(), 0);
    buffer_size_ = size;

    begin_ = current_begin_ = current_end_ = readed_ = buffer_.data();
    end_ = buffer_.data() + size - 1;  // 버퍼의 마지막 위치로 수정
    recycle_ = buffer_.data() + size - 1;

    used_size_ = processed_size_ = 0;
    return true;
}

void RingBuffer::Reset()
{
    CriticalSection::Lock lock(critical_section_);
    std::fill(buffer_.begin(), buffer_.end(), 0);
    //std::memset(buffer_.data(), 0, buffer_size_);

    begin_ = current_begin_ = current_end_ = readed_ = buffer_.data();
    end_ = buffer_.data() + buffer_size_ - 1;  // 버퍼의 마지막 위치로 수정
    recycle_ = buffer_.data() + buffer_size_ - 1;

    used_size_ = processed_size_ = 0;
}

char* RingBuffer::GetBuffer(size_t requiredSize) 
{
    CriticalSection::Lock lock(critical_section_);

    if (used_size_ + requiredSize > buffer_size_) 
    {
        throw std::runtime_error("RingBuffer::GetBuffer: Insufficient buffer space");
    }

    if (static_cast<size_t>(end_ - current_end_) >= requiredSize)
    {
        current_begin_ = current_end_;
        current_end_ += requiredSize;
    }
    else 
    {
        if (begin_ + requiredSize > current_begin_)
        {
            return nullptr;
        }
        current_begin_ = begin_;
        current_end_ = begin_ + requiredSize;
    }

    used_size_ += requiredSize;
    return current_begin_;
}

char* RingBuffer::GetBuffer(size_t movePos, size_t processedMove, size_t requireSize) 
{
    CriticalSection::Lock lock(critical_section_);

    if (movePos + requireSize > buffer_size_) 
    {
        return nullptr;
    }

    if (static_cast<size_t>(end_ - current_begin_) > movePos + requireSize) 
    {
        current_begin_ += movePos;
        current_end_ = current_begin_ + requireSize;
        readed_ += processedMove;
    }
    else 
    {
        if (begin_ + requireSize >= current_end_) 
        {
            return nullptr;
        }

        size_t moveAmount = static_cast<size_t>(current_begin_ - readed_);
        std::memmove(begin_, readed_, movePos + moveAmount);

        current_begin_ = begin_ + movePos;
        current_end_ = current_begin_ + requireSize;
        readed_ = begin_ + processedMove;
    }

    std::memset(current_begin_, 0, Constants::Network::MAX_PACKET_SIZE);
    return current_begin_;
}

void RingBuffer::ReleaseBuffer(size_t releaseSize) 
{
    CriticalSection::Lock lock(critical_section_);
    used_size_ -= releaseSize;
}
