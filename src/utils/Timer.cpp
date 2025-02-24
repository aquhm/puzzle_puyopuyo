#include "Timer.hpp"

Timer::Timer() 
{
    Reset();
}

void Timer::Start() 
{
    if (is_stopped_) 
    {
        // 일시 정지된 시간을 보정
        paused_time_ = Clock::now() - pause_time_;
        last_time_ = Clock::now();
        is_stopped_ = false;
    }
}

void Timer::Stop() 
{
    if (!is_stopped_) 
    {
        pause_time_ = Clock::now();
        is_stopped_ = true;
    }
}

void Timer::Reset() 
{
    start_time_ = Clock::now();
    last_time_ = start_time_;
    pause_time_ = start_time_;
    is_stopped_ = false;
    total_time_ = Duration::zero();
    paused_time_ = Duration::zero();
}

float Timer::GetElapsedTime() {
    if (is_stopped_) {
        return 0.0f;
    }

    TimePoint current_time = Clock::now();
    Duration delta_time = current_time - last_time_;
    last_time_ = current_time;

    total_time_ += delta_time;
    return delta_time.count();
}

std::chrono::duration<float> Timer::GetTotalTime() const {
    if (is_stopped_) {
        return pause_time_ - start_time_ - paused_time_;
    }

    return Clock::now() - start_time_ - paused_time_;
}