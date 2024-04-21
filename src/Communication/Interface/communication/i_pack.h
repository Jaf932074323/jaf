#pragma once
#include <memory>
#include "comm_struct.h"
#include "i_channel.h"

namespace jaf
{
namespace comm
{

// 处理通信通道通信通道 负责从通道读写数据
class IPack
{
public:
	IPack(){}
	virtual ~IPack() {};

public:
	// 获取对应的通道
	virtual std::shared_ptr<IChannel> GetChannel() = 0;
	virtual SConstData GetData() = 0;
};

}
}