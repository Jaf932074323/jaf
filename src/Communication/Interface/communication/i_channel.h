#pragma once
#include "util/co_coroutine.h"

namespace jaf
{
namespace comm
{

// 通信通道
class IChannel
{
public:
	IChannel(){}
	virtual ~IChannel() {};

public:
	virtual Coroutine<bool> Start() = 0;
	virtual Coroutine<bool> Read(unsigned char* buff, size_t buff_size, size_t* recv_len) = 0;
	virtual Coroutine<bool> Write(const unsigned char* buff, size_t buff_size) = 0;
	virtual void Stop() = 0;
};

}
}