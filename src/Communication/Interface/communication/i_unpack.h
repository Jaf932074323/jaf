#pragma once
#include <memory>
#include "util/co_coroutine.h"
#include "i_channel.h"
#include "i_deal_pack.h"

namespace jaf
{
namespace comm
{

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class IUnpack
{
public:
	IUnpack(){}
	virtual ~IUnpack() {};

public:
	virtual Coroutine<void> Run(std::shared_ptr <IChannel> channel, std::shared_ptr<IDealPack> deal_pack) = 0;
};

}
}