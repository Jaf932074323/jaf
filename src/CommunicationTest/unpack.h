#pragma once
#include <memory>
#include "util/co_coroutine.h"
#include "Interface/communication/i_unpack.h"
#include "impl/recv_buffer.h"

// 处理通信通道通信通道 负责从通道读写数据
class Unpack: public jaf::comm::IUnpack
{
public:
	Unpack(){}
	virtual ~Unpack() {};

public:
	virtual jaf::Coroutine<void> Run(std::shared_ptr <jaf::comm::IChannel> channel, std::shared_ptr<jaf::comm::IDealPack> deal_pack) override;

};
