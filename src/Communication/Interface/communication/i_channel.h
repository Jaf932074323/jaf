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
    enum class EState
    {
        CRS_SUCCESS    = 0, // 成功
        CRS_CHANNEL_END          = 1, // 通道结束运行
        CRS_FAIL = 2,         // 失败,具体错误原因由code_表示
        CRS_CHANNEL_DISCONNECTED    = 3, // 通道断开连接
        CRS_TIMEOUT  = 4, // 超时
        CRS_UNKNOWN, // 未知错误
    };
    
    EState state = EState::CRS_UNKNOWN;
    size_t len   = 0;     // 处理长度
    int code_ = 0; // 错误代码 state为CRS_FAIL时有效
    std::string error;    // 当失败且未超时，失败原因
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