#pragma once
#include "util/co_coroutine.h"
#include "Interface/communication/i_channel_user.h"

typedef void* HANDLE;

namespace jaf
{
namespace comm
{

// windows平台下的完成端口实现的一种通信协议接口
class IIocpUser
{
public:
	IIocpUser(){}
	virtual ~IIocpUser(){}

public:
	virtual void SetChannelUser(std::shared_ptr<IChannelUser> user) =0;
	virtual jaf::Coroutine<void> Run(HANDLE completion_handle) = 0;
	virtual void Stop() = 0;
};

}
}