#pragma once
#include "Interface/communication/i_channel_user.h"
#include "util/co_coroutine.h"
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

// TCP·þÎñ¶Ë
class ITcpServer
{
public:
    virtual ~ITcpServer(){};

public:
    virtual void SetAddr(const std::string& ip, uint16_t port)      = 0;
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) = 0;
    virtual void SetAcceptCount(size_t accept_count)                = 0;
    virtual void SetMaxClientCount(size_t max_client_count)         = 0;
    virtual Coroutine<void> Run()                                   = 0;
    virtual void Stop()                                             = 0;
};

} // namespace comm
} // namespace jaf