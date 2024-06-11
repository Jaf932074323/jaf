#pragma once
#include "i_channel.h"
#include "util/co_coroutine.h"
#include <memory>

namespace jaf
{
namespace comm
{

// ͨ��ʹ����
class IChannelUser
{
public:
    IChannelUser() {}
    virtual ~IChannelUser(){};

public:
    // ����ͨ��
    virtual Coroutine<void> Access(std::shared_ptr<IChannel> channel) = 0;
};

} // namespace comm
} // namespace jaf