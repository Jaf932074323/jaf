#pragma once
#include "Interface/communication/i_unpack.h"
#include "impl/recv_buffer.h"
#include "util/co_coroutine.h"
#include <memory>

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class Unpack : public jaf::comm::IUnpack
{
public:
    Unpack() {}
    virtual ~Unpack(){};

public:
    virtual jaf::Coroutine<void> Run(std::shared_ptr<jaf::comm::IChannel> channel, std::shared_ptr<jaf::comm::IDealPack> deal_pack) override;
};
