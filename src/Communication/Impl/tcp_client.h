#pragma once
#include <string>
#include "util/co_coroutine.h"
#include "Interface/communication/i_channel_user.h"
#include "Interface/communication/i_unpack.h"

namespace jaf
{
namespace comm
{

// TCP客户端
class TcpClient
{
public:
	TcpClient(std::string local_ip, uint16_t local_port, std::string remote_ip, uint16_t remote_port);
	virtual ~TcpClient();

public:
	virtual void SetChannelUser(std::shared_ptr<IChannelUser> user);
	virtual jaf::Coroutine<void> Run(HANDLE completion_handle);
	virtual void Stop();

private:
	void Init(void);
	jaf::Coroutine<void> Run();

private:
    class ConnectAwaitable
    {
    public:
        ConnectAwaitable(TcpClient* client);
        ~ConnectAwaitable();
        bool await_ready();
        bool await_suspend(std::coroutine_handle<> co_handle);
		SOCKET await_resume();
        void IoCallback(IOCP_DATA* pData);
    private:
        TcpClient* client_ = nullptr;
        SOCKET socket_ = INVALID_SOCKET; // 连接套接字
        IOCP_DATA iocp_data_;
        std::coroutine_handle<> handle_;

        char buf_[1] = { 0 };
        DWORD dwBytes_ = 0;
    };
private:
	bool run_flag_ = false;

	HANDLE completion_handle_ = nullptr;

	std::string local_ip_ = "0.0.0.0";
	uint16_t local_port_ = 0;
	std::string remote_ip_;
	uint16_t remote_port_ = 0;

	std::shared_ptr<IChannelUser> user_ = nullptr; // 通道使用者
};

}
}