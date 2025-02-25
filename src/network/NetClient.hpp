#pragma once
/*
 *
 * 설명: 클라이언트 패킷 처리 WSAEventSelect
 *
 */

#include "NetCommon.hpp"
#include "../core/common/constants/Constants.hpp"

#include <string>
#include <array>
#include <memory>
#include <thread>
#include <span>


class NetClient 
{
public:
    NetClient() = default;
    virtual ~NetClient();

    NetClient(const NetClient&) = delete;
    NetClient& operator=(const NetClient&) = delete;

    // 초기화 및 제어 함수들
    [[nodiscard]] virtual bool Start(HWND hwnd);
    virtual void Exit();


    [[nodiscard]] bool Connect(std::string_view ip, uint16_t port);
    void Disconnect(bool force = false);

    void SendData(std::span<const char> data);
    [[nodiscard]] bool ProcessRecv(WPARAM wParam, LPARAM lParam);    
    
protected:
    virtual void ProcessPacket(std::span<const char> packet) = 0;
    virtual void ProcessConnectExit() {};

    void EventPollingThreadFunc();
    //void HandleNetworkEvent(SOCKET socket, LONG event_type);

private:
    // 워커 스레드 관련 
    //static unsigned int CALLBACK WorkerThread(void* arg);
    //[[nodiscard]] bool CreateWorkerThread();
    //unsigned int RunWorkerThread();

    // 내부 헬퍼 함수
    [[nodiscard]] bool InitSocket();
    void LogError(std::wstring_view msg) const;

private:
    WSASession wsa_session_;
    Socket socket_;
    HANDLE worker_thread_{ nullptr };

    HWND hwnd_;
    bool initialize_;

    std::array<char, Constants::Network::CLIENT_BUF_SIZE * 8> msg_buffer_{};
    uint32_t recv_remain_size_{ 0 };

    std::atomic<bool> is_connected_{ false };

    WSAEVENT event_handle_{ WSA_INVALID_EVENT };

    std::thread event_polling_thread_;
    std::atomic<bool> polling_thread_running_{ false };    
};
