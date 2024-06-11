#pragma once
#include "i_channel.h"
#include "util/co_coroutine.h"
#include <memory>

namespace jaf
{
namespace comm
{

// 通道使用者
class IChannelUser
{
public:
    IChannelUser() {}
    virtual ~IChannelUser(){};

public:
    // 操作通道
    virtual Coroutine<void> Access(std::shared_ptr<IChannel> channel) = 0;
};

} // namespace comm
} // namespace jaf