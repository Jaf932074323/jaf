#pragma once
#include "util/co_coroutine.h"
#include <string>

namespace jaf
{
namespace comm
{

// 通信通道执行结果
struct SChannelResult
{
    bool success = false; // true成功,false失败
    size_t len   = 0;     // 处理长度
    bool timeout = false; // true超时,false未超时
    std::string error;    // 当失败时，失败原因
};

// 通信通道
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