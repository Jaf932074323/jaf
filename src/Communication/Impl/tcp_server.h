#pragma once
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <mutex>
#include "i_iocp_user.h"
#include "Interface/communication/i_unpack.h"
#include "Interface/communication/i_deal_pack.h"

namespace jaf
{
namespace comm
{

// TCP服务端
class TcpServer:public IIocpUser
{
public:
	TcpServer(std::string ip, uint16_t port);
	virtual ~TcpServer();

public:
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) override;
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
        TcpServer* server = nullptr;
        SOCKET listen_socket_ = 0; // 侦听套接字
        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle_;

        SOCKET sock_{ INVALID_SOCKET };
        char buf_[1] = { 0 };
        DWORD dwBytes_ = 0;
    };

private:
    bool run_flag_ = false;

	HANDLE completion_handle_ = nullptr;
	SOCKET listen_socket_ = 0; // 侦听套接字

	std::string ip_ = "0.0.0.0";
	uint16_t port_ = 0;

    std::shared_ptr<IChannelUser> user_ = nullptr; // 通道使用者

    std::mutex channels_mutex_; // 所有通道的同步锁
    std::map<std::string,std::shared_ptr<IChannel>> channels_; // 当前连接的所有通道 key由IP和端口

};

}
}