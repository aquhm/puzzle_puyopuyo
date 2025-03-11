#include "NetClient.hpp"
#include "../core/common/constants/Constants.hpp"


#include <format>
#include <span>
#include "NetworkController.hpp"
#include "../utils/Logger.hpp"

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

        hwnd_ = hwnd;

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

        polling_thread_running_ = true;
        event_polling_thread_ = std::thread(&NetClient::EventPollingThreadFunc, this);

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
    polling_thread_running_ = false;
    if (event_polling_thread_.joinable()) 
    {
        event_polling_thread_.join();
    }

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
        char* packet = msg_buffer_.data();

        // 버퍼 오버플로우 방지
        if (recv_remain_size_ >= Constants::Network::CLIENT_BUF_SIZE)
        {
            LogError(L"Buffer overflow prevented");
            recv_remain_size_ = 0;
            return false;
        }

        // 데이터 수신
        recv_size = recv(socket_.get(), msg_buffer_.data() + recv_remain_size_,
            Constants::Network::CLIENT_BUF_SIZE - recv_remain_size_, 0);

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

        // 패킷 처리 루프
        packet = msg_buffer_.data();
        while (recv_remain_size_ >= sizeof(PacketBase))
        {
            // 패킷 기본 정보 읽기
            PacketBase packetBase;
            std::memcpy(&packetBase, packet, sizeof(PacketBase));

            // 유효성 검사 
            if (packetBase.size < sizeof(PacketBase) || packetBase.size > Constants::Network::MAX_PACKET_SIZE)
            {
                // LogError(std::format("Invalid packet size: {}", baseHeader.size));
                recv_remain_size_ = 0; // 버퍼 비우기
                return false;
            }

            // 전체 패킷이 수신되었는지 확인
            if (recv_remain_size_ < packetBase.size)
            {
                break; // 더 많은 데이터 필요
            }

            // 패킷 처리
            ProcessPacket(std::span<const char>(packet, packetBase.size));

            // 버퍼 포인터 및 남은 크기 업데이트
            recv_remain_size_ -= packetBase.size;
            packet += packetBase.size;
        }

        // 남은 데이터 이동
        if (recv_remain_size_ > 0 && packet != msg_buffer_.data())
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

void NetClient::EventPollingThreadFunc() 
{
    while (polling_thread_running_) 
    {
        DWORD result = WSAWaitForMultipleEvents(1, &event_handle_, FALSE, 100, FALSE);

        if (result == WSA_WAIT_EVENT_0) 
        {
            WSANETWORKEVENTS network_events;
            if (WSAEnumNetworkEvents(socket_.get(), event_handle_, &network_events) == 0) 
            {
                if (network_events.lNetworkEvents & FD_READ) 
                {
                    if (network_events.iErrorCode[FD_READ_BIT] == 0) 
                    {
                        // 읽기 이벤트 처리
                        if (ProcessRecv(static_cast<WPARAM>(socket_.get()), static_cast<LPARAM>(FD_READ)) == false)
                        {
                            LOGGER.Error("ProcessRecv Failed");
                        }
                    }
                }

                if (network_events.lNetworkEvents & FD_CLOSE) 
                {
                    // 연결 종료 이벤트 처리
                    if (ProcessRecv(static_cast<WPARAM>(socket_.get()), static_cast<LPARAM>(FD_CLOSE)) == false)
                    {
						LOGGER.Error("ProcessRecv Failed");
                    }
                }
                //// SDL 이벤트 생성
                //SDL_Event sdl_event;
                //sdl_event.type = SDL_EVENT_USER;  // SDL3에서는 SDL_EVENT_USER, SDL2에서는 SDL_USEREVENT
                //sdl_event.user.code = Constants::Network::NETWORK_EVENT_CODE;

                //// 소켓 핸들과 네트워크 이벤트 플래그를 데이터로 전달
                //sdl_event.user.data1 = reinterpret_cast<void*>(socket_.get());
                //sdl_event.user.data2 = reinterpret_cast<void*>(static_cast<uintptr_t>(network_events.lNetworkEvents));

                //// SDL 이벤트 큐에 이벤트 추가
                //SDL_PushEvent(&sdl_event);

                //// 이벤트 재설정
                //WSAResetEvent(event_handle_);
            }
        }
    }
}
