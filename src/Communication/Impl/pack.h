#pragma once
#include "Interface/communication/i_pack.h"

namespace jaf
{
namespace comm
{

// 处理通信通道通信通道 负责从通道读写数据
class Pack: public IPack
{
public:
	Pack(std::shared_ptr<IChannel> channel, std::shared_ptr<unsigned char[]> buff, SConstData data);
	virtual ~Pack();;

public:
	// 获取对应的通道
	virtual std::shared_ptr<IChannel> GetChannel() override;
	virtual SConstData GetData() override;

private:
	std::shared_ptr<IChannel> channel_;
	std::shared_ptr<unsigned char[]> buff_;
	SConstData data_;
};

}
}