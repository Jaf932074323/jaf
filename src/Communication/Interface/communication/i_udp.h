#pragma once
#include "Interface/communication/i_channel_user.h"
#include "util/co_coroutine.h"
#include <memory>
#include <string>

namespace jaf
{
namespace comm
{

class IUdp
{
public:
    virtual ~IUdp(){};

public:
    virtual void SetAddr(const std::string& local_ip, uint16_t local_port, const std::string& remote_ip, uint16_t remote_port) = 0;
    virtual void SetChannelUser(std::shared_ptr<IChannelUser> user)                                                            = 0;
    virtual Coroutine<void> Run()                                                                                         = 0;
    virtual void Stop()                                                                                                        = 0;
};

} // namespace comm
} // namespace jaf