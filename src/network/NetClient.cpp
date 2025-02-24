#include "NetClient.hpp"
#include "../core/common/constants/Constants.hpp"


#include <format>
#include <span>
#include "NetworkController.hpp"

NetClient::~NetClient()
{
    Exit();

    if (event_handle_ != WSA_INVALID_EVENT) 
    {
        WSACloseEvent(event_handle_);
        event_handle_ = WSA_INVALID_EVENT;
    }
}

bool NetClient::Start(HWND hwnd)
{
    try
    {
        if (initialize_ == true)
        {
            return false;
        }

        hwnd_ = hwnd; // WSAAsyncSelect 메시지를 받을 윈도우 핸들 설정

        if (InitSocket() == false)
        {
            throw NetworkException("InitSocket Failed");
        }

        if (Connect(NETWORK.GetAddress(), Constants::Network::NET_PORT) == false)
        {
            throw NetworkException("Connect Failed");
            return false;
        }

        recv_remain_size_ = 0;
        msg_buffer_.fill(0);

        initialize_ = true;

        return true;
    }
    catch (const NetworkException& e)
    {
        LogError(std::wstring(e.what(), e.what() + strlen(e.what())));
        return false;
    }
}

bool NetClient::InitSocket()
{
    /*WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        throw NetworkException("WSAStartup Failed");
    }*/

    socket_ = Socket(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED));

    if (!socket_.is_valid())
    {
        throw NetworkException("WSASocket Failed");
    }

    // TCP_NODELAY 옵션 설정
    BOOL no_delay = TRUE;
    if (setsockopt(socket_.get(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&no_delay), sizeof(no_delay)) == SOCKET_ERROR)
    {
        throw NetworkException("setsockopt TCP_NODELAY Failed");
    }

    return true;
}

bool NetClient::Connect(std::string_view ip, uint16_t port)
{
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.data(), &server_addr.sin_addr) <= 0)
    {
        throw NetworkException("inet_pton Failed: Invalid IP address or conversion error");
    }

    int retVal = 0;
    BOOL bNoDelay = TRUE;
    
    if (retVal = setsockopt(socket_.get(), IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay)); retVal == SOCKET_ERROR)
    {
        throw NetworkException("setsockopt Failed");
    }

    if (retVal = connect(socket_.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)); retVal == SOCKET_ERROR)
    {
        int errorCode = WSAGetLastError();
        throw NetworkException("connect Failed: Error code " + std::to_string(errorCode));
    }

    if (event_handle_ != WSA_INVALID_EVENT) 
    {
        WSACloseEvent(event_handle_);
    }

    event_handle_ = WSACreateEvent();
    if (event_handle_ == WSA_INVALID_EVENT) 
    {
        throw NetworkException("WSACreateEvent Failed");
    }

    if (WSAEventSelect(socket_.get(), event_handle_, FD_READ | FD_CLOSE) == SOCKET_ERROR) 
    {
        WSACloseEvent(event_handle_);
        event_handle_ = WSA_INVALID_EVENT;
        throw NetworkException("WSAEventSelect Failed");
    }

    is_connected_ = true;
    return true;
}

void NetClient::SendData(std::span<const char> data)
{
    if (!is_connected_ || data.empty())
    {
        return;
    }

    const int result = send(socket_.get(), data.data(), static_cast<int>(data.size()), 0);

    if (result == SOCKET_ERROR)
    {
        LogError(L"send Failed");
    }
}

void NetClient::Exit()
{
    if (is_connected_)
    {
        is_connected_ = false;

        if (event_handle_ != WSA_INVALID_EVENT) 
        {
            WSACloseEvent(event_handle_);
            event_handle_ = WSA_INVALID_EVENT;
        }

        Disconnect();
    }
}

void NetClient::Disconnect(bool force)
{
    if (!socket_.is_valid())
    {
        return;
    }

    linger optLinger = { force ? 1U : 0U, 0U };

    shutdown(socket_.get(), SD_BOTH);
    setsockopt(socket_.get(), SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&optLinger), sizeof(optLinger));

    socket_.close();
    is_connected_ = false;
}

bool NetClient::ProcessRecv(WPARAM wParam, LPARAM lParam)
{
    const int event = WSAGETSELECTEVENT(lParam);
    const int error = WSAGETSELECTERROR(lParam);

    if (error != 0)
    {
        Disconnect();
        LogError(L"WSAGETSELECTERROR");
        return false;
    }

    switch (event)
    {
    case FD_READ:
    {
        uint32_t recv_size = 0;
        uint32_t packet_size = 0;
        char* packet = msg_buffer_.data();

        recv_size = recv(socket_.get(), msg_buffer_.data() + recv_remain_size_, NetworkConfig::BUFFER_SIZE, 0);

        if (recv_size == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                Disconnect();
                LogError(L"recv Failed");
                return false;
            }
        }
        else if (recv_size == 0)
        {
            Disconnect();
            LogError(L"Connection closed");
            return false;
        }

        recv_remain_size_ += recv_size;

        if (recv_remain_size_ < NetworkConfig::PACKET_SIZE_LENGTH)
        {
            return false;
        }

        packet = msg_buffer_.data();
        while (true)
        {
            memcpy(&packet_size, packet, NetworkConfig::PACKET_SIZE_LENGTH);

            if (recv_remain_size_ < packet_size || packet_size <= 0)
            {
                break;
            }

            ProcessPacket(std::span<const char>(packet, packet_size));

            recv_remain_size_ -= packet_size;
            packet += packet_size;

            if (recv_remain_size_ <= 0 || recv_remain_size_ < NetworkConfig::PACKET_SIZE_LENGTH)
            {
                break;
            }
        }

        if (recv_remain_size_ > 0)
        {
            memmove(msg_buffer_.data(), packet, recv_remain_size_);
        }
        break;
    }

    case FD_CLOSE:
        ProcessConnectExit();
        break;
    }

    return true;
}

void NetClient::LogError(std::wstring_view msg) const
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&lpMsgBuf),
        0,
        nullptr
    );

    OutputDebugString(static_cast<LPCWSTR>(lpMsgBuf));
    LocalFree(lpMsgBuf);
}
