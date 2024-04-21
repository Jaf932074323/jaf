#pragma once
#include "Interface/communication/i_deal_pack.h"

// 处理通信通道通信通道 负责从通道读写数据
class DealPack: public jaf::comm::IDealPack
{
public:
	DealPack(){}
	virtual ~DealPack() {};

public:
	// 处理包
	virtual void Deal(std::shared_ptr<jaf::comm::IPack> pack) override;
};
