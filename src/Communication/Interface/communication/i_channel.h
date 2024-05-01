#pragma once
#include "util/co_coroutine.h"
#include <string>

namespace jaf
{
namespace comm
{

// ͨ��ͨ��ִ�н��
struct SChannelResult
{
    bool success = false; // true�ɹ�,falseʧ��
    size_t len   = 0;     // ������
    bool timeout = false; // true��ʱ,falseδ��ʱ
    std::string error;    // ��ʧ��ʱ��ʧ��ԭ��
};

// ͨ��ͨ��
class IChannel
{
public:
    IChannel() {}
    virtual ~IChannel(){};

public:
    virtual Coroutine<bool> Start()                                                                        = 0;
    virtual void Stop()                                                                                    = 0;
    virtual Coroutine<SChannelResult> Read(unsigned char* buff, size_t buff_size, uint64_t timeout)        = 0;
    virtual Coroutine<SChannelResult> Write(const unsigned char* buff, size_t buff_size, uint64_t timeout) = 0;
};

} // namespace comm
} // namespace jaf