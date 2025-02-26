#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <memory>
#include <source_location>
#include <format>
#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <Windows.h>

#include <SDL3/SDL_log.h>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger {
public:
    static Logger& GetInstance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    static void SDLLogOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message);
    void InitializeSDLLogging();

    template<typename... Args>
    void Debug(std::format_string<Args...> fmt, Args&&... args);

    template<typename... Args>
    void Info(std::format_string<Args...> fmt, Args&&... args);

    template<typename... Args>
    void Warning(std::format_string<Args...> fmt, Args&&... args);

    template<typename... Args>
    void Error(std::format_string<Args...> fmt, Args&&... args);

    template<typename... Args>
    void Critical(std::format_string<Args...> fmt, Args&&... args);

    // 초기화 및 정리
    bool Initialize(const std::filesystem::path& logDir = "logs");
    void Shutdown();

    // 설정
    void SetLogToConsole(bool enable) { log_to_console_ = enable; }
    void SetLogToFile(bool enable) { log_to_file_ = enable; }
    void SetLogToDebugger(bool enable) { log_to_debugger_ = enable; }
    void SetLogLevel(LogLevel level) { log_level_ = level; }

    // SDL 로그 레벨에 맞는 함수들
    void SDLLogVerbose(int category, const char* fmt, ...);
    void SDLLogDebug(int category, const char* fmt, ...);
    void SDLLogInfo(int category, const char* fmt, ...);
    void SDLLogWarn(int category, const char* fmt, ...);
    void SDLLogError(int category, const char* fmt, ...);
    void SDLLogCritical(int category, const char* fmt, ...);

private:
    Logger() = default;
    ~Logger() = default;

    // 내부 로깅 구현
    template<typename... Args>
    void Log(LogLevel level,
        const std::source_location& location,
        std::format_string<Args...> fmt,
        Args&&... args);

    // 로그 파일 관리
    void RotateLogFiles();
    [[nodiscard]] std::filesystem::path GetCurrentLogFilePath() const;

    // 유틸리티 함수
    [[nodiscard]] static std::string GetTimestamp();
    [[nodiscard]] static std::string_view LogLevelToString(LogLevel level);

    // 멤버 변수
    std::filesystem::path log_directory_;
    std::unique_ptr<std::FILE, decltype(&std::fclose)> current_log_file_{ nullptr, std::fclose };

    LogLevel log_level_{ LogLevel::Info };
    bool log_to_console_{ true };
    bool log_to_file_{ true };
    bool log_to_debugger_{ true };

    static constexpr size_t MAX_LOG_FILES = 5;
    static constexpr size_t MAX_LOG_SIZE = 10 * 1024 * 1024; // 10MB
};

// 템플릿 구현
template<typename... Args>
void Logger::Debug(std::format_string<Args...> fmt, Args&&... args) 
{
    Log(LogLevel::Debug, std::source_location::current(), fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::Info(std::format_string<Args...> fmt, Args&&... args) 
{
    Log(LogLevel::Info, std::source_location::current(), fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::Warning(std::format_string<Args...> fmt, Args&&... args) 
{
    Log(LogLevel::Warning, std::source_location::current(), fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::Error(std::format_string<Args...> fmt, Args&&... args) 
{
    Log(LogLevel::Error, std::source_location::current(), fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::Critical(std::format_string<Args...> fmt, Args&&... args) 
{
    Log(LogLevel::Critical, std::source_location::current(), fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::Log(LogLevel level,
    const std::source_location& location,
    std::format_string<Args...> fmt,
    Args&&... args) 
{
    if (level < log_level_)
    {
        return;
    }

    try {
        // 로그 메시지 생성
        std::string message = std::format("[{}] [{}] [{}:{}] {}\n",
            GetTimestamp(),
            LogLevelToString(level),
            location.file_name(),
            location.line(),
            std::format(fmt, std::forward<Args>(args)...));

        // 콘솔 출력
        if (log_to_console_) 
        {
            std::printf("%s", message.c_str());
            std::fflush(stdout);
        }

        // 파일 출력
        if (log_to_file_ && current_log_file_) 
        {
            std::fprintf(current_log_file_.get(), "%s", message.c_str());
            std::fflush(current_log_file_.get());
        }

        // 디버거 출력
        if (log_to_debugger_) 
        {
            OutputDebugStringA(message.c_str());
        }
    }
    catch (const std::exception& e) 
    {
        // 로깅 실패 시 최소한의 에러 출력
        std::fprintf(stderr, "Logging failed: %s\n", e.what());
    }
}

// 글로벌 액세스 매크로
#define LOGGER Logger::GetInstance()

// 편의성 매크로
#define LOG_DEBUG(...) LOGGER.Debug(__VA_ARGS__)
#define LOG_INFO(...) LOGGER.Info(__VA_ARGS__)
#define LOG_WARNING(...) LOGGER.Warning(__VA_ARGS__)
#define LOG_ERROR(...) LOGGER.Error(__VA_ARGS__)
#define LOG_CRITICAL(...) LOGGER.Critical(__VA_ARGS__)

#define SDL_LOG_VERBOSE(category, ...) LOGGER.SDLLogVerbose(category, __VA_ARGS__)
#define SDL_LOG_DEBUG(category, ...) LOGGER.SDLLogDebug(category, __VA_ARGS__)
#define SDL_LOG_INFO(category, ...) LOGGER.SDLLogInfo(category, __VA_ARGS__)
#define SDL_LOG_WARN(category, ...) LOGGER.SDLLogWarn(category, __VA_ARGS__)
#define SDL_LOG_ERROR(category, ...) LOGGER.SDLLogError(category, __VA_ARGS__)
#define SDL_LOG_CRITICAL(category, ...) LOGGER.SDLLogCritical(category, __VA_ARGS__)