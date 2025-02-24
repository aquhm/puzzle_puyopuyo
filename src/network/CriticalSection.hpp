#pragma once

#include <WinSock2.h>
#include <utility>
#include <cstring>

class CriticalSection 
{
public:
    CriticalSection() noexcept 
    {
        InitializeCriticalSection(&cs_);
    }

    ~CriticalSection() noexcept 
    {
        DeleteCriticalSection(&cs_);
    }

    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;

    CriticalSection(CriticalSection&& other) noexcept : cs_{} 
    {
        std::memcpy(&cs_, &other.cs_, sizeof(CRITICAL_SECTION));
        std::memset(&other.cs_, 0, sizeof(CRITICAL_SECTION));
    }

    CriticalSection& operator=(CriticalSection&& other) noexcept 
    {
        if (this != &other) 
        {
            DeleteCriticalSection(&cs_);
            std::memcpy(&cs_, &other.cs_, sizeof(CRITICAL_SECTION));
            std::memset(&other.cs_, 0, sizeof(CRITICAL_SECTION));
        }
        return *this;
    }


    class Lock 
    {
    public:
        explicit Lock(CriticalSection& cs) noexcept : cs_(&cs) 
        {
            cs_->enter();
        }

        Lock(Lock&& other) noexcept : cs_(std::exchange(other.cs_, nullptr)) {}

        ~Lock() noexcept 
        {
            if (cs_)
            {
                cs_->leave();
            }
        }

        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;

        Lock& operator=(Lock&& other) noexcept 
        {
            if (this != &other) 
            {
                if (cs_)
                {
                    cs_->leave();
                }
                cs_ = std::exchange(other.cs_, nullptr);
            }
            return *this;
        }

    private:
        CriticalSection* cs_;
    };

    void enter() noexcept 
    {
        EnterCriticalSection(&cs_);
    }

    void leave() noexcept 
    {
        LeaveCriticalSection(&cs_);
    }

    [[nodiscard]] bool try_enter() noexcept 
    {
        return TryEnterCriticalSection(&cs_) != 0;
    }

private:
    CRITICAL_SECTION cs_;
};