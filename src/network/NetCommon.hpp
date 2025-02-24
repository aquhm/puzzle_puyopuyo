#pragma once

#include <winSock2.h>
#include <WS2tcpip.h>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>


#define WM_SOCKET (WM_USER + 1)

// 상수 정의를 enum class로 현대화
enum class OperationType : uint8_t
{
    Receive,
    Send
};

// 네트워크 설정 상수
struct NetworkConfig
{
    static constexpr uint16_t DEFAULT_PORT = 9000;
    static constexpr size_t PACKET_SIZE_LENGTH = sizeof(uint32_t);
    static constexpr size_t MAX_PACKET_SIZE = 256;
    static constexpr size_t MAX_CLIENTS = 4;
    static constexpr uint32_t BUFFER_SIZE = 1024;
    static constexpr size_t MAX_WORKER_THREADS = 1;
};

// OVERLAPPED 구조체 현대화 
struct OverlappedEx
{
    WSAOVERLAPPED overlapped{};
    SOCKET socket{ INVALID_SOCKET };
    WSABUF wsa_buf{};
    char* begin_buf{ nullptr };
    OperationType operation{ OperationType::Receive };
    int packet_size{ 0 };
    int remain_size{ 0 };
    int receive_size{ 0 };

    OverlappedEx() = default;
};

// RAII 소켓 래퍼 클래스
class Socket
{
    SOCKET socket_{ INVALID_SOCKET };

public:
    Socket() = default;
    explicit Socket(SOCKET socket) : socket_(socket) {}
    ~Socket() 
    {
        if (is_valid()) {
            closesocket(socket_);
        }
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept : socket_(other.socket_) 
    {
        other.socket_ = INVALID_SOCKET;
    }

    Socket& operator=(Socket&& other) noexcept 
    {
        if (this != &other) 
        {
            if (is_valid())
            {
                closesocket(socket_);
            }
            socket_ = other.socket_;
            other.socket_ = INVALID_SOCKET;
        }
        return *this;
    }

    [[nodiscard]] SOCKET get() const { return socket_; }
    [[nodiscard]] bool is_valid() const { return socket_ != INVALID_SOCKET; }

    void close()
    {
        if (is_valid())
        {
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
    }
};

// 네트워크 에러 처리를 위한 예외 클래스
class NetworkException : public std::runtime_error
{
public:
    explicit NetworkException(const std::string& message)
        : std::runtime_error(message) {}
};


class WSASession
{
public:
    WSASession()
    {
        WSADATA wsa_data;
        const int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != 0) 
        {
            throw NetworkException("WSAStartup Failed");
        }
    }

    ~WSASession()
    {
        WSACleanup();
    }

    WSASession(const WSASession&) = delete;
    WSASession& operator=(const WSASession&) = delete;
};

// Result 타입
template<typename T>
class Result
{
    bool success_;
    std::string error_message_;
    T value_;

public:
    explicit Result(T value) : success_(true), value_(std::move(value)) {}
    explicit Result(std::string error) : success_(false), error_message_(std::move(error)) {}

    [[nodiscard]] bool is_success() const { return success_; }
    [[nodiscard]] const std::string& get_error() const { return error_message_; }
    [[nodiscard]] const T& get_value() const { return value_; }
};
