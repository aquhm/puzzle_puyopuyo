#include "Logger.hpp"
#include <Windows.h>
#include <format>
#include <iostream>

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

bool Logger::Initialize(const std::filesystem::path& logDir) 
{
    try 
    {
        log_directory_ = logDir;
        std::filesystem::create_directories(log_directory_);
        RotateLogFiles();

        auto logPath = GetCurrentLogFilePath();
        FILE* file = nullptr;
        if (_wfopen_s(&file, logPath.wstring().c_str(), L"a") != 0) {
            std::cerr << "Failed to open log file: " << logPath << std::endl;
            return false;
        }

        current_log_file_.reset(file);
        return true;
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void Logger::Shutdown() 
{
    if (current_log_file_) {
        current_log_file_.reset();
    }
}

void Logger::RotateLogFiles() 
{
    try 
    {
        auto oldestLog = log_directory_ / std::format("log_{}.txt", MAX_LOG_FILES - 1);
        if (std::filesystem::exists(oldestLog)) {
            std::filesystem::remove(oldestLog);
        }

        // Rotate existing logs
        for (int i = MAX_LOG_FILES - 2; i >= 0; --i) {
            auto currentLog = log_directory_ / std::format("log_{}.txt", i);
            if (std::filesystem::exists(currentLog)) {
                auto newPath = log_directory_ / std::format("log_{}.txt", i + 1);
                std::filesystem::rename(currentLog, newPath);
            }
        }
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Log rotation failed: " << e.what() << std::endl;
    }
}

std::filesystem::path Logger::GetCurrentLogFilePath() const 
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    return log_directory_ / std::format("log_{}_{:02d}_{:02d}_{:02d}.txt",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
}

std::string Logger::GetTimestamp() 
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
}

std::string_view Logger::LogLevelToString(LogLevel level) 
{
    switch (level) 
    {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARNING";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Critical: return "CRITICAL";
    default: return "UNKNOWN";
    }
}