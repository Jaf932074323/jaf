#pragma once
#include <memory>
#include "i_pack.h"

namespace jaf
{
namespace comm
{

// 处理通信通道通信通道 负责从通道读写数据
class IDealPack
{
public:
	IDealPack(){}
	virtual ~IDealPack() {};

public:
	// 处理包
	virtual void Deal(std::shared_ptr<IPack> pack) = 0;
};

}
}