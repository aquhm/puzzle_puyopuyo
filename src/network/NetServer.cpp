#include "NetServer.hpp"

#include <format>
#include <process.h>
#include "../utils/Logger.hpp"

NetServer::NetServer() :
    clients_(std::make_unique<ClientInfo[]>(Constants::Network::MAX_CLIENT))
{
}

NetServer::~NetServer()
{
    ExitServer();
}

bool NetServer::StartServer()
{
    try
    {
        if (InitSocket() == false)
        {
            throw NetworkException("InitSocket Failed");
        }

        if (BindAndListen(Constants::Network::NET_PORT) == false)
        {
            throw NetworkException("BindAndListen Failed");
        }

        if (CreateThreadAndIOCP() == false)
        {
            throw NetworkException("CreateThreadAndIOCP Failed");
        }

        return true;
    }
    catch (const NetworkException& e)
    {
        LogError(std::wstring(e.what(), e.what() + strlen(e.what())));
        return false;
    }
}

bool NetServer::InitSocket()
{
    listen_socket_ = Socket(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED));

    if (!listen_socket_.is_valid())
    {
        throw NetworkException("WSASocket Failed");
    }

    // TCP_NODELAY 옵션 설정
    BOOL no_delay = TRUE;
    if (setsockopt(listen_socket_.get(), IPPROTO_TCP, TCP_NODELAY,
        reinterpret_cast<char*>(&no_delay), sizeof(no_delay)) == SOCKET_ERROR)
    {
        throw NetworkException("setsockopt Failed");
    }

    return true;
}

bool NetServer::BindAndListen(uint16_t port)
{
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_socket_.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR)
    {
        throw NetworkException("bind Failed");
    }

    if (listen(listen_socket_.get(), 5) == SOCKET_ERROR)
    {
        throw NetworkException("listen Failed");
    }

    return true;
}

bool NetServer::CreateThreadAndIOCP()
{
    // IOCP 생성
    iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

    if (!iocp_handle_)
    {
        return false;
    }

    worker_running_ = true;
    accepter_running_ = true;

    // 워커 스레드 생성
    if (!CreateWorkerThread())
    {
        return false;
    }

    // Accept 스레드 생성
    if (!CreateAccepterThread())
    {
        return false;
    }

    return true;
}

bool NetServer::CreateWorkerThread() 
{
    for (size_t i = 0; i < Constants::Network::MAX_WORKERTHREAD; ++i) 
    {
        unsigned int thread_id = 0;
        worker_threads_[i] = (HANDLE)_beginthreadex(
            nullptr,
            0,
            &CallWorkerThread,  // & 연산자 추가
            this,
            CREATE_SUSPENDED,
            &thread_id
        );

        if (!worker_threads_[i]) {
            return false;
        }

        ResumeThread(worker_threads_[i]);
    }
    return true;
}

bool NetServer::CreateAccepterThread() 
{
    unsigned int thread_id = 0;
    accepter_thread_ = (HANDLE)_beginthreadex(
        nullptr,
        0,
        &CallAccepterThread,  // & 연산자 추가
        this,
        CREATE_SUSPENDED,
        &thread_id
    );

    if (!accepter_thread_) {
        return false;
    }

    ResumeThread(accepter_thread_);
    return true;
}

unsigned int CALLBACK NetServer::CallWorkerThread(void* arg)
{
    auto* server = static_cast<NetServer*>(arg);
    return server->WorkerThread();
}

unsigned int CALLBACK NetServer::CallAccepterThread(void* arg)
{
    auto* server = static_cast<NetServer*>(arg);
    return server->AccepterThread();
}

unsigned int NetServer::WorkerThread() 
{
    while (worker_running_) 
    {
        DWORD bytes_transferred = 0;
        ULONG_PTR completion_key = 0;
        LPOVERLAPPED overlapped = nullptr;

        const BOOL result = GetQueuedCompletionStatus(
            iocp_handle_,
            &bytes_transferred,
            &completion_key,
            &overlapped,
            INFINITE
        );

        auto* client = reinterpret_cast<ClientInfo*>(completion_key);
        if (!client) 
        {
            worker_running_ = false;
            return 0;
        }

        auto* overlapped_ex = reinterpret_cast<OverlappedEx*>(overlapped);
        if (!overlapped_ex) 
        {
            continue;
        }

        if (!result || (result && bytes_transferred == 0)) 
        {
            DisconnectProcess(client);
            continue;
        }

        // 작업 타입에 따른 처리
        switch (overlapped_ex->operation) 
        {
        case OperationType::Receive:
            ProcessRecv(client, overlapped_ex, bytes_transferred);
            break;
        case OperationType::Send:
            ProcessSend(client, overlapped_ex, bytes_transferred);
            break;
        }
    }

    return 0;
}

unsigned int NetServer::AccepterThread() 
{
    sockaddr_in client_addr{};
    int addr_len = sizeof(client_addr);

    while (accepter_running_) 
    {
        ClientInfo* client = GetEmptyClientInfo();
        if (!client) {
            continue;
        }

        client->socket = Socket(accept(listen_socket_.get(),
            reinterpret_cast<sockaddr*>(&client_addr), &addr_len));

        if (client->socket.is_valid() == false) 
        {
            continue;
        }

        if (BindIOCP(client) == false) 
        {
            continue;
        }

        client->recv_buffer.Reset();

        if (BindRecv(client, 0, 0) == false)
        {
            continue;
        }

        ++client_count_;
        ConnectProcess(client);
    }

    return 0;
}

void NetServer::ProcessRecv(ClientInfo* client, OverlappedEx* overlapped, DWORD bytes) 
{
    if (!client || !overlapped)
        return;

    overlapped->receive_size += bytes;

    char* processed_pos = client->recv_buffer.GetProcessedPos();
    if (!processed_pos) 
    {
        return;
    }

    // 패킷 크기 정보보다 작은 경우 재수신
    if (overlapped->receive_size < Constants::Network::PACKET_SIZE_LEN) 
    {
        if (BindRecv(client, processed_pos, overlapped->receive_size) == false)
        {
            LOGGER.Error("BindRecv Failed");
        }
        return;
    }

    // 패킷 크기 확인 및 처리
    int packet_size = 0;
    memcpy(&packet_size, processed_pos, Constants::Network::PACKET_SIZE_LEN);

    int remain_size = overlapped->receive_size;

    if (packet_size <= remain_size) 
    {
        if (!PacketProcess(client, std::span<const char>(overlapped->begin_buf, packet_size))) {
            CloseSocket(client);
            return;
        }

        processed_pos = overlapped->begin_buf;
        remain_size -= packet_size;
        processed_pos += packet_size;

        while (true) 
        {
            if (remain_size >= Constants::Network::PACKET_SIZE_LEN) 
            {
                memcpy(&packet_size, processed_pos, Constants::Network::PACKET_SIZE_LEN);

                if (packet_size <= 0 || static_cast<size_t>(packet_size) > client->recv_buffer.GetBufferSize()) 
                {
             
                    LogError(L"Invalid packet size");
                    CloseSocket(client);
                    return;
                }

                overlapped->packet_size = packet_size;

                // 완전한 패킷이 수신된 경우
                if (packet_size <= remain_size) 
                {
                    if (!PacketProcess(client, std::span<const char>(processed_pos, packet_size))) 
                    {
                        CloseSocket(client);
                        return;
                    }

                    remain_size -= packet_size;
                    processed_pos += packet_size;
                }
                else 
                {
                    break;
                }
            }
            else 
            {
                break;
            }
        }
    }

    // 다음 수신 작업 등록
    if (BindRecv(client, processed_pos, remain_size) == false)
    {
		LOGGER.Error("BindRecv Failed");
    }
}

void NetServer::ProcessSend(ClientInfo* client, OverlappedEx* overlapped, DWORD bytes) 
{
    if (!client || !overlapped) 
    {
        return;
    }

    uint16_t packet_type = 0;
    if (overlapped->wsa_buf.buf && overlapped->wsa_buf.len >= Constants::Network::PACKET_SIZE_LEN + sizeof(uint16_t)) 
    {
        memcpy(&packet_type, overlapped->wsa_buf.buf + Constants::Network::PACKET_SIZE_LEN, sizeof(uint16_t));
    }

    // 전송 완료된 바이트 수를 누적
    overlapped->remain_size += bytes;

    // 전송 완료된 데이터를 큐에서 제거
    std::shared_ptr<SendQueueData> temp_buffer;
    client->send_queue.try_pop(temp_buffer);

    // 다음 전송 데이터가 있으면 처리
    if (!client->send_queue.empty())
    {
        std::shared_ptr<SendQueueData> next_data;
        if (client->send_queue.try_pop(next_data))
        {
            // 다음 전송 데이터로 오버랩 구조체 설정
            client->send_overlapped.operation = OperationType::Send;
            client->send_overlapped.remain_size = 0;
            client->send_overlapped.packet_size = static_cast<int>(next_data->buffer.size());
            client->send_overlapped.wsa_buf.buf = next_data->buffer.data();
            client->send_overlapped.wsa_buf.len = static_cast<ULONG>(next_data->buffer.size());
            client->send_overlapped.begin_buf = next_data->buffer.data();

            ZeroMemory(&client->send_overlapped.overlapped, sizeof(OVERLAPPED));

            // 다음 전송 시작
            DWORD sent_bytes = 0;
            WSASend(
                client->socket.get(),
                &client->send_overlapped.wsa_buf,
                1,
                &sent_bytes,
                0,
                &client->send_overlapped.overlapped,
                nullptr
            );
        }
    }
}

bool NetServer::BindRecv(ClientInfo* client, char* processed_pos, int remain_size) 
{
    if (!client || !client->socket.is_valid())
    {
        return false;
    }

    char* buffer = nullptr;

    if (processed_pos == 0 && remain_size == 0) 
    {
        buffer = client->recv_buffer.GetBuffer(Constants::Network::MAX_PACKET_SIZE);
    }
    else 
    {
        int move_pos = static_cast<int>(processed_pos - client->recv_buffer.GetCurrentBeginPos()) + remain_size;
        int process_move = static_cast<int>(processed_pos - client->recv_buffer.GetCurrentBeginPos());

        if (move_pos < 0 || process_move < 0) 
        {
            buffer = client->recv_buffer.GetBuffer(Constants::Network::MAX_PACKET_SIZE);
        }
        else 
        {
            buffer = client->recv_buffer.GetBuffer(move_pos, process_move, Constants::Network::MAX_PACKET_SIZE);
        }
    }

    if (!buffer) 
    {
        return false;
    }

    // WSARecv 작업을 위한 설정
    client->recv_overlapped.wsa_buf.len = static_cast<ULONG>(Constants::Network::MAX_PACKET_SIZE);
    client->recv_overlapped.packet_size = Constants::Network::MAX_PACKET_SIZE;
    client->recv_overlapped.wsa_buf.buf = buffer;
    client->recv_overlapped.begin_buf = client->recv_buffer.GetProcessedPos();
    client->recv_overlapped.receive_size = remain_size;
    client->recv_overlapped.operation = OperationType::Receive;

    DWORD flags = 0;
    DWORD recv_bytes = 0;

    ZeroMemory(&client->recv_overlapped.overlapped, sizeof(OVERLAPPED));

    int result = WSARecv(
        client->socket.get(),
        &client->recv_overlapped.wsa_buf,
        1,
        &recv_bytes,
        &flags,
        &client->recv_overlapped.overlapped,
        nullptr
    );

    if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) 
    {
        LogError(L"WSARecv()");
        return false;
    }

    return true;
}

bool NetServer::SendMsg(ClientInfo* client, std::span<const char> msg)
{
    if (!client || !client->socket.is_valid() || msg.empty())
    {
        return false;
    }

    // 전송 데이터 생성
    auto send_data = std::make_shared<SendQueueData>(msg);

    // 오버랩 구조체 설정
    client->send_overlapped.operation = OperationType::Send;
    client->send_overlapped.remain_size = 0;
    client->send_overlapped.packet_size = static_cast<int>(send_data->buffer.size());
    client->send_overlapped.wsa_buf.buf = send_data->buffer.data();
    client->send_overlapped.wsa_buf.len = static_cast<ULONG>(send_data->buffer.size());
    client->send_overlapped.begin_buf = send_data->buffer.data();

    client->send_queue.push(send_data);

    ZeroMemory(&client->send_overlapped.overlapped, sizeof(OVERLAPPED));

    DWORD sent_bytes = 0;
    const int result = WSASend(
        client->socket.get(),
        &client->send_overlapped.wsa_buf,
        1,
        &sent_bytes,
        0,
        &client->send_overlapped.overlapped,
        nullptr
    );

    if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        LogError(L"WSASend()");
        DisconnectProcess(client);
        return false;
    }

    return true;
}

void NetServer::CloseSocket(ClientInfo* client, bool force) 
{
    if (!client || !client->socket.is_valid()) 
    {
        return;
    }

    linger opt_linger = 
    {
        force ? 1U : 0U,  // l_onoff
        0U               // l_linger
    };

    // socketClose 전 설정
    shutdown(client->socket.get(), SD_BOTH);
    setsockopt(client->socket.get(),
        SOL_SOCKET,
        SO_LINGER,
        reinterpret_cast<char*>(&opt_linger),
        sizeof(opt_linger));

    // 버퍼 초기화
    client->recv_buffer.Reset();

    std::shared_ptr<SendQueueData> dummy;
    while (client->send_queue.try_pop(dummy)) {}

    // 오버랩 구조체 초기화
    ZeroMemory(&client->recv_overlapped, sizeof(OverlappedEx));
    ZeroMemory(&client->send_overlapped, sizeof(OverlappedEx));

    client->socket.close();
    --client_count_;
}

bool NetServer::ExitServer() 
{
    DestroyThread();

    for (size_t i = 0; i < Constants::Network::MAX_CLIENT; ++i) 
    {
        if (clients_[i].socket.is_valid()) 
        {
            CloseSocket(&clients_[i]);
        }
    }

    return true;
}

void NetServer::DestroyThread() {
    if (iocp_handle_) {
        worker_running_ = false;

        // WorkerThread 종료
        for (size_t i = 0; i < Constants::Network::MAX_WORKERTHREAD; ++i) {
            PostQueuedCompletionStatus(iocp_handle_, 0, 0, nullptr);
        }

        for (auto& handle : worker_threads_) {
            if (handle) {
                WaitForSingleObject(handle, INFINITE);
                CloseHandle(handle);
                handle = nullptr;
            }
        }

        CloseHandle(iocp_handle_);
        iocp_handle_ = nullptr;
    }

    // AccepterThread 종료
    if (accepter_running_) {
        accepter_running_ = false;

        if (listen_socket_.is_valid()) {
            listen_socket_.close();
        }

        if (accepter_thread_) {
            WaitForSingleObject(accepter_thread_, INFINITE);
            CloseHandle(accepter_thread_);
            accepter_thread_ = nullptr;
        }
    }
}

ClientInfo* NetServer::GetEmptyClientInfo() 
{
    for (size_t i = 0; i < Constants::Network::MAX_CLIENT; ++i)
    {
        if (!clients_[i].socket.is_valid()) 
        {
            return &clients_[i];
        }
    }
    return nullptr;
}

bool NetServer::BindIOCP(ClientInfo* client) 
{
    HANDLE handle = CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(client->socket.get()),
        iocp_handle_,
        reinterpret_cast<ULONG_PTR>(client),
        0
    );

    if (!handle || handle != iocp_handle_) 
    {
        return false;
    }

    return true;
}

void NetServer::LogError(std::wstring_view msg) const 
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

    // 여기서는 간단히 OutputDebugString으로 처리
    // 실제로는 로깅 시스템을 사용하는 것이 좋습니다
    OutputDebugString(static_cast<LPCWSTR>(lpMsgBuf));
    LocalFree(lpMsgBuf);
}
