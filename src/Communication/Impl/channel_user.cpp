#include "channel_user.h"

namespace jaf
{
namespace comm
{

ChannelUser::ChannelUser(std::shared_ptr<IUnpack> unpack, std::shared_ptr<IDealPack> deal_pack)
	: unpack_(unpack)
	, deal_pack_(deal_pack)
{
}

ChannelUser::~ChannelUser()
{
}

Coroutine<void> ChannelUser::Access(std::shared_ptr<IChannel> channel)
{
	co_await unpack_->Run(channel, deal_pack_);
	co_return;
}

}
}