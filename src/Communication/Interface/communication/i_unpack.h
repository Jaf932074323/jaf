#pragma once
#include "i_channel.h"
#include "i_deal_pack.h"
#include "util/co_coroutine.h"
#include <memory>

namespace jaf
{
namespace comm
{

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
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