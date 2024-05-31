#pragma once
#include "i_iocp_user.h"
#include "interface/communication/i_deal_pack.h"
#include "interface/communication/i_unpack.h"
#include "util/co_await.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace jaf
{
namespace comm
{

// TCP�����
class TcpServer : public IIocpUser
{
public:
    TcpServer(std::string ip, uint16_t port);
    virtual ~TcpServer();

public:
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) override;
    virtual void SetAcceptCount(size_t accept_count) override;
    virtual void SetMaxClientCount(size_t max_client_count) override;
    virtual jaf::Coroutine<void> Run(HANDLE completion_handle) override;
    virtual void Stop() override;

private:
    void Init(void);
    jaf::Coroutine<void> Accept();
    jaf::Coroutine<void> RunSocket(SOCKET socket);

private:
    class AcceptAwaitable
    {
    public:
        AcceptAwaitable(TcpServer* server, SOCKET listen_socket);
        ~AcceptAwaitable();
        bool await_ready();
        bool await_suspend(std::coroutine_handle<> co_handle);
        SOCKET await_resume();
        void IoCallback(IOCP_DATA* pData);

    private:
        TcpServer* server     = nullptr;
        SOCKET listen_socket_ = 0; // �����׽���
        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle_;

        SOCKET sock_{INVALID_SOCKET};
        char buf_[1]   = {0};
        DWORD dwBytes_ = 0;
    };

private:
    bool run_flag_ = false;
    CoAwait await_stop_;

    HANDLE completion_handle_ = nullptr;
    SOCKET listen_socket_     = 0; // �����׽���

    std::string ip_ = "0.0.0.0";
    uint16_t port_  = 0;

    std::shared_ptr<IChannelUser> user_ = nullptr; // ͨ��ʹ����
    size_t accept_count_                = 5;
    size_t max_client_count_            = SOMAXCONN; // ���ͻ�����������

    std::mutex channels_mutex_;                                 // ����ͨ����ͬ����
    std::map<std::string, std::shared_ptr<IChannel>> channels_; // ��ǰ���ӵ�����ͨ�� key��IP�Ͷ˿�
};

} // namespace comm
} // namespace jaf