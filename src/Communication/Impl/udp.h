#pragma once
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <mutex>
#include "i_iocp_user.h"
#include "Interface/communication/i_unpack.h"

namespace jaf
{
namespace comm
{

class Udp
{
public:
	Udp(std::string local_ip, uint16_t local_port, std::string remote_ip, uint16_t remote_port);
	virtual ~Udp();

public:
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user);
	virtual jaf::Coroutine<void> Run(HANDLE completion_handle);
	virtual void Stop();

private:
    void Init(void);
    jaf::Coroutine<void> RunSocket();

private:
    bool run_flag_ = false;

	HANDLE completion_handle_ = nullptr;
	SOCKET socket_ = 0; // 侦听套接字

	std::string local_ip_ = "0.0.0.0";
	uint16_t local_port_ = 0;
	std::string remote_ip_;
	uint16_t remote_port_ = 0;

    std::shared_ptr<IChannelUser> user_ = nullptr; // 通道使用者

};

}
}