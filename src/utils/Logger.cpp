#include "Logger.hpp"
#include <Windows.h>
#include <format>
#include <iostream>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>

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

        InitializeSDLLogging();

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

// SDL 로그를 Logger 클래스로 라우팅하는 콜백 함수
void Logger::SDLLogOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    Logger& logger = *static_cast<Logger*>(userdata);

    // SDL 로그 우선순위를 Logger 로그 레벨로 변환
    LogLevel level;
    switch (priority) {
    case SDL_LOG_PRIORITY_VERBOSE:
    case SDL_LOG_PRIORITY_DEBUG:
        level = LogLevel::Debug;
        break;
    case SDL_LOG_PRIORITY_INFO:
        level = LogLevel::Info;
        break;
    case SDL_LOG_PRIORITY_WARN:
        level = LogLevel::Warning;
        break;
    case SDL_LOG_PRIORITY_ERROR:
        level = LogLevel::Error;
        break;
    case SDL_LOG_PRIORITY_CRITICAL:
        level = LogLevel::Critical;
        break;
    default:
        level = LogLevel::Info;
        break;
    }

    // 카테고리 이름 얻기
    std::string categoryName;
    switch (category) {
    case SDL_LOG_CATEGORY_APPLICATION: categoryName = "APPLICATION"; break;
    case SDL_LOG_CATEGORY_ERROR: categoryName = "ERROR"; break;
    case SDL_LOG_CATEGORY_ASSERT: categoryName = "ASSERT"; break;
    case SDL_LOG_CATEGORY_SYSTEM: categoryName = "SYSTEM"; break;
    case SDL_LOG_CATEGORY_AUDIO: categoryName = "AUDIO"; break;
    case SDL_LOG_CATEGORY_VIDEO: categoryName = "VIDEO"; break;
    case SDL_LOG_CATEGORY_RENDER: categoryName = "RENDER"; break;
    case SDL_LOG_CATEGORY_INPUT: categoryName = "INPUT"; break;
    case SDL_LOG_CATEGORY_TEST: categoryName = "TEST"; break;
    default: categoryName = "CUSTOM"; break;
    }

    std::string formattedMessage = std::format("[SDL-{}] {}", categoryName, message);

    auto location = std::source_location::current();

    switch (level) 
    {
    case LogLevel::Debug:
        logger.Debug("{}", formattedMessage);
        break;
    case LogLevel::Info:
        logger.Info("{}", formattedMessage);
        break;
    case LogLevel::Warning:
        logger.Warning("{}", formattedMessage);
        break;
    case LogLevel::Error:
        logger.Error("{}", formattedMessage);
        break;
    case LogLevel::Critical:
        logger.Critical("{}", formattedMessage);
        break;
    }
}


void Logger::InitializeSDLLogging() 
{
    SDL_SetLogOutputFunction(SDLLogOutputFunction, this);

    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
}

void Logger::SDLLogVerbose(int category, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(category, SDL_LOG_PRIORITY_VERBOSE, fmt, args);
    va_end(args);
}

void Logger::SDLLogDebug(int category, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(category, SDL_LOG_PRIORITY_DEBUG, fmt, args);
    va_end(args);
}

void Logger::SDLLogInfo(int category, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(category, SDL_LOG_PRIORITY_INFO, fmt, args);
    va_end(args);
}

void Logger::SDLLogWarn(int category, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(category, SDL_LOG_PRIORITY_WARN, fmt, args);
    va_end(args);
}

void Logger::SDLLogError(int category, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(category, SDL_LOG_PRIORITY_ERROR, fmt, args);
    va_end(args);
}

void Logger::SDLLogCritical(int category, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(category, SDL_LOG_PRIORITY_CRITICAL, fmt, args);
    va_end(args);
}