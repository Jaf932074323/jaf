#pragma once
#include "i_channel.h"
#include "i_deal_pack.h"
#include "util/co_coroutine.h"
#include <memory>

namespace jaf
{
namespace comm
{

// 处理通信通道通信通道 负责从通道读写数据
class IUnpack
{
public:
    IUnpack() {}
    virtual ~IUnpack(){};

public:
    virtual Coroutine<void> Run(std::shared_ptr<IChannel> channel, std::shared_ptr<IDealPack> deal_pack) = 0;
};

} // namespace comm
} // namespace jaf