#pragma once
#include "Interface/communication/i_channel_user.h"
#include "Interface/communication/i_udp.h"
#include "interface/communication/i_unpack.h"
#include "iocp_head.h"
#include "time_head.h"
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

class Udp : public IUdp
{
public:
    Udp(IGetCompletionPort* get_completion_port, std::shared_ptr<jaf::time::ITimer> timer = nullptr);
    virtual ~Udp();

public:
    virtual void SetAddr(const std::string& local_ip, uint16_t local_port, const std::string& remote_ip, uint16_t remote_port);
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user);
    virtual Coroutine<void> Run();
    virtual void Stop();

private:
    void Init(void);


private:
    IGetCompletionPort* get_completion_port_ = nullptr;
    HANDLE completion_handle_ = nullptr;
    SOCKET socket_            = 0; // 侦听套接字

    std::shared_ptr<jaf::time::ITimer> timer_;

    std::string local_ip_  = "0.0.0.0";
    uint16_t local_port_   = 0;
    std::string remote_ip_ = "0.0.0.0";
    uint16_t remote_port_  = 0;

    std::shared_ptr<IChannelUser> user_      = nullptr; // 通道使用者
    std::mutex channel_mutex_;
    bool run_flag_                     = false;
    std::shared_ptr<IChannel> channel_ = nullptr;
};

} // namespace comm
} // namespace jaf