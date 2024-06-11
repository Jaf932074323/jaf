#pragma once
#include "comm_struct.h"
#include "i_channel.h"
#include <memory>

namespace jaf
{
namespace comm
{

// ����ͨ��ͨ��ͨ��ͨ�� �����ͨ����д����
class IPack
{
public:
    IPack() {}
    virtual ~IPack(){};

public:
    // ��ȡ��Ӧ��ͨ��
    virtual std::shared_ptr<IChannel> GetChannel() = 0;
    virtual SConstData GetData()                   = 0;
};

} // namespace comm
} // namespace jaf