#pragma once
#include "interface/communication/i_channel_user.h"
#include "interface/communication/i_unpack.h"
#include "util/co_coroutine.h"

namespace jaf
{
namespace comm
{

// ͨ��ʹ����
class ChannelUser : public IChannelUser
{
public:
    ChannelUser(std::shared_ptr<IUnpack> unpack, std::shared_ptr<IDealPack> deal_pack);
    virtual ~ChannelUser();
    ;

public:
    // ����ͨ��
    virtual Coroutine<void> Access(std::shared_ptr<IChannel> channel) override;

private:
    std::shared_ptr<IUnpack> unpack_      = nullptr;
    std::shared_ptr<IDealPack> deal_pack_ = nullptr;
};

} // namespace comm
} // namespace jaf