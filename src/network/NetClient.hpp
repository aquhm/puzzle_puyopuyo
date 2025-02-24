#pragma once
/*
 *
 * ����: Ŭ���̾�Ʈ ��Ŷ ó�� WSAEventSelect
 *
 */

#include "NetCommon.hpp"

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

    // �ʱ�ȭ �� ���� �Լ���
    [[nodiscard]] virtual bool Start(HWND hwnd);
    virtual void Exit();

    // ��Ʈ��ũ ���
    void SendData(std::span<const char> data);
    [[nodiscard]] bool ProcessRecv(WPARAM wParam, LPARAM lParam);

    // ���� ����
    [[nodiscard]] bool Connect(std::string_view ip, uint16_t port);
    void Disconnect(bool force = false);


protected:
    virtual void ProcessPacket(std::span<const char> packet) = 0;
    virtual void ProcessConnectExit() {};

private:
    // ��Ŀ ������ ���� 
    //static unsigned int CALLBACK WorkerThread(void* arg);
    //[[nodiscard]] bool CreateWorkerThread();
    //unsigned int RunWorkerThread();

    // ���� ���� �Լ�
    [[nodiscard]] bool InitSocket();
    void LogError(std::wstring_view msg) const;

private:
    WSASession wsa_session_;            // RAII WSA ����
    Socket socket_;                     // RAII ���� ����
    HANDLE worker_thread_{ nullptr };

    HWND hwnd_;
    bool initialize_;

    std::array<char, NetworkConfig::BUFFER_SIZE * 8> msg_buffer_{};
    uint32_t recv_remain_size_{ 0 };

    std::atomic<bool> is_connected_{ false };

    WSAEVENT event_handle_{ WSA_INVALID_EVENT };
};
