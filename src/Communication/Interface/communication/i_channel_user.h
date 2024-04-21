#pragma once
#include <memory>
#include "util/co_coroutine.h"
#include "i_channel.h"

namespace jaf
{
namespace comm
{

// ͨ��ʹ����
class IChannelUser
{
public:
	IChannelUser() {}
	virtual ~IChannelUser() {};

public:
	// ����ͨ��
	virtual Coroutine<void> Access(std::shared_ptr<IChannel> channel) = 0;
};

}
}