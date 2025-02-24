#pragma once
#include <chrono>

class Timer 
{

public:
    Timer();
    ~Timer() = default;

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) noexcept = default;
    Timer& operator=(Timer&&) noexcept = default;

    void Start();
    void Stop();
    void Reset();

    [[nodiscard]] float GetElapsedTime();
    [[nodiscard]] bool IsRunning() const noexcept { return !is_stopped_; }
    [[nodiscard]] std::chrono::duration<float> GetTotalTime() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<float>;

    TimePoint start_time_{};
    TimePoint last_time_{};
    TimePoint pause_time_{};

    bool is_stopped_{ true };
    Duration total_time_{};
    Duration paused_time_{};
};