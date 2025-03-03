#pragma once

#include <functional>
#include <vector>
#include <chrono>
#include <utility>
#include <algorithm>
#include <memory>

class TimerScheduler 
{

    using TimerId = size_t;

private:
    struct Task
    {
        TimerId id;
        std::chrono::steady_clock::time_point executeTime;
        std::function<void()> callback;
    };

    TimerScheduler() : nextTimerId_(1) {}
    ~TimerScheduler() = default;

    TimerScheduler(const TimerScheduler&) = delete;
    TimerScheduler& operator=(const TimerScheduler&) = delete;
    TimerScheduler(TimerScheduler&&) = delete;
    TimerScheduler& operator=(TimerScheduler&&) = delete;

public:
    

    static TimerScheduler& GetInstance() 
    {
        static TimerScheduler instance;
        return instance;
    }

    // 지정된 시간(초) 후에 콜백 실행 예약
    TimerId ScheduleTask(float delayInSeconds, std::function<void()> callback) 
    {
        auto executeTime = std::chrono::steady_clock::now() +
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<float>(delayInSeconds));

        TimerId newId = nextTimerId_++;

        scheduledTasks_.push_back({ newId, executeTime, std::move(callback) });

        return newId;
    }

    // 예약된 작업 취소
    bool CancelTask(TimerId id) 
    {
        auto it = std::find_if(scheduledTasks_.begin(), scheduledTasks_.end(),
            [id](const Task& task) 
            {
                return task.id == id; 
            });

        if (it != scheduledTasks_.end()) 
        {
            scheduledTasks_.erase(it);
            return true;
        }

        return false;
    }

    void Update() 
    {
        auto now = std::chrono::steady_clock::now();

        std::vector<std::function<void()>> tasksToExecute;

        auto newEnd = std::remove_if(scheduledTasks_.begin(), scheduledTasks_.end(),
            [&](const Task& task) 
            {
                if (task.executeTime <= now) 
                {
                    tasksToExecute.push_back(task.callback);
                    return true;
                }
                return false;
            });

        scheduledTasks_.erase(newEnd, scheduledTasks_.end());

        for (const auto& callback : tasksToExecute) 
        {
            callback();
        }
    }

private:
    std::vector<Task> scheduledTasks_;
    TimerId nextTimerId_;
};

#define TIMER_SCHEDULER TimerScheduler::GetInstance()