#pragma once
#include "Interface/communication/i_channel_user.h"
#include "util/co_coroutine.h"
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// TCP客户端
class ITcpClient
{
public:
    virtual ~ITcpClient() {};

public:
    virtual void SetAddr(const std::string& remote_ip, uint16_t remote_port, const std::string& local_ip = "0.0.0.0", uint16_t local_port = 0) = 0;
    // 设置连接时间
    // connect_timeout 连接超时时间
    // reconnect_wait_time 重连等待时间
    virtual void SetConnectTime(uint64_t connect_timeout, uint64_t reconnect_wait_time) = 0;
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) = 0;
    virtual jaf::Coroutine<void> Run() = 0;
    virtual void Stop() = 0;
};

} // namespace comm
} // namespace jaf